package cmd

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"net/http"
	"os"
	"os/signal"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
	"syscall"
	"time"

	"github.com/STommydx/cp-templates/tools/ccli/statement"
	"github.com/STommydx/cp-templates/tools/ccli/statement/adapters/hkoi"
	"github.com/charmbracelet/glamour"
	"github.com/charmbracelet/glamour/styles"
	huma "github.com/danielgtaylor/huma/v2"
	"github.com/danielgtaylor/huma/v2/adapters/humachi"
	"github.com/go-chi/chi/v5"
	"github.com/mattn/go-isatty"
	"github.com/spf13/cobra"
	"golang.org/x/term"
	"gopkg.in/yaml.v3"
)

var (
	statementOutputDir string
	statementAdapter   string
	statementPort      int
	statementFormat    string
	statementCapture   string
	statementFile      string
)

// defaultStatementWidth is the wrapping width when the terminal size is unknown.
const defaultStatementWidth = 80

const maxConcurrentCaptures = 4

var statementCmd = &cobra.Command{
	Use:   "statement",
	Short: "Capture and inspect problem statements",
	Args:  usageArgs(cobra.NoArgs),
}

var serveStatementCmd = &cobra.Command{
	Use:   "serve",
	Short: "Serve the local PageMole statement receiver",
	Args:  usageArgs(cobra.NoArgs),
	RunE: func(cmd *cobra.Command, _ []string) error {
		return runStatementServer(cmd)
	},
}

var statementRootCmd = &cobra.Command{
	Use:   "root",
	Short: "Print the statement storage root",
	Args:  usageArgs(cobra.NoArgs),
	RunE: func(cmd *cobra.Command, _ []string) error {
		root, err := statement.ResolveRoot(statementOutputDir)
		if err != nil {
			return err
		}
		_, err = fmt.Fprintln(cmd.OutOrStdout(), root)
		return err
	},
}

var statementListCmd = &cobra.Command{
	Use:   "list",
	Short: "List captured statements",
	Args:  usageArgs(cobra.NoArgs),
	RunE: func(cmd *cobra.Command, _ []string) error {
		return runStatementList(cmd)
	},
}

var statementPathCmd = &cobra.Command{
	Use:   "path <adapter/code-or-urlhash>",
	Short: "Print a captured statement path",
	Args:  usageArgs(cobra.ExactArgs(1)),
	RunE: func(cmd *cobra.Command, args []string) error {
		return runStatementPath(cmd, args[0])
	},
}

var statementShowCmd = &cobra.Command{
	Use:   "show <adapter/code-or-urlhash>",
	Short: "Print a captured statement",
	Args:  usageArgs(cobra.ExactArgs(1)),
	RunE: func(cmd *cobra.Command, args []string) error {
		return runStatementShow(cmd, args[0])
	},
}

type captureInput struct {
	Body    statement.CaptureEnvelope
	RawBody []byte
}

// Resolve enforces the domain invariant that captures reference hierarchical pages.
func (i *captureInput) Resolve(_ huma.Context) []error {
	u, err := i.Body.ParsedURL()
	if err != nil || u.Host == "" {
		return []error{&huma.ErrorDetail{Location: "body.url", Message: "url must be an absolute hierarchical URL"}}
	}
	return nil
}

type captureResponse struct {
	Mode      string   `json:"mode" doc:"Parser mode"`
	Directory string   `json:"directory" doc:"Capture directory relative to storage root"`
	Capture   string   `json:"capture" doc:"Raw capture path relative to storage root"`
	Metadata  string   `json:"metadata" doc:"Metadata path relative to storage root"`
	Statement string   `json:"statement" doc:"Markdown path relative to storage root"`
	Warnings  []string `json:"warnings,omitempty" doc:"Non-fatal parser warnings"`
}
type captureOutput struct {
	Body captureResponse
}

func runStatementServer(cmd *cobra.Command) error {
	if statementPort < 1 || statementPort > 65535 {
		return usageError("port must be between 1 and 65535")
	}
	adapters := registeredStatementAdapters()
	if statementAdapter != "" && findStatementAdapter(adapters, statementAdapter) == nil {
		return usageError("unknown adapter %q", statementAdapter)
	}
	root, err := statement.ResolveRoot(statementOutputDir)
	if err != nil {
		return err
	}
	if err := statement.EnsureRoot(root); err != nil {
		return fmt.Errorf("create statement root: %w", err)
	}
	if err := statement.CleanupStaging(root); err != nil {
		return fmt.Errorf("clean statement staging: %w", err)
	}

	router := chi.NewRouter()
	api := humachi.New(router, huma.DefaultConfig("ccli statement server", "1.0.0"))
	captureSlots := make(chan struct{}, maxConcurrentCaptures)
	registerStatementAPI(api, root, adapters, statementAdapter, captureSlots)
	server := &http.Server{
		Addr:              fmt.Sprintf("127.0.0.1:%d", statementPort),
		Handler:           router,
		ReadHeaderTimeout: 5 * time.Second,
		ReadTimeout:       30 * time.Second,
		WriteTimeout:      30 * time.Second,
		IdleTimeout:       60 * time.Second,
		MaxHeaderBytes:    64 << 10,
	}
	return serveUntilSignal(server)
}

func registeredStatementAdapters() []statement.Adapter {
	return []statement.Adapter{hkoi.New()}
}

func findStatementAdapter(adapters []statement.Adapter, id string) statement.Adapter {
	for _, adapter := range adapters {
		if adapter.ID() == id {
			return adapter
		}
	}
	return nil
}

func registerStatementAPI(api huma.API, root string, adapters []statement.Adapter, forcedAdapter string, captureSlots chan struct{}) {
	huma.Register[captureInput, captureOutput](api, huma.Operation{
		OperationID:   "capture-statement-html",
		Method:        http.MethodPost,
		Path:          "/",
		Summary:       "Capture rendered PageMole statement HTML",
		Description:   "Accept a PageMole statement-html version 1 envelope and persist immutable raw, metadata, and Markdown artifacts.",
		DefaultStatus: http.StatusCreated,
		MaxBodyBytes:  statement.MaxCaptureBytes + 1,
		Errors:        []int{http.StatusBadRequest, http.StatusRequestEntityTooLarge, http.StatusUnsupportedMediaType, http.StatusUnprocessableEntity, http.StatusInternalServerError, http.StatusServiceUnavailable},
	}, func(ctx context.Context, input *captureInput) (*captureOutput, error) {
		if captureSlots != nil {
			select {
			case captureSlots <- struct{}{}:
				defer func() { <-captureSlots }()
			case <-ctx.Done():
				return nil, huma.Error503ServiceUnavailable("capture processing capacity is unavailable")
			}
		}
		capture := input.Body.WithRawBytes(input.RawBody)
		result, err := statement.ParseCapture(&capture, adapters, statement.ParseOptions{ForcedAdapterID: forcedAdapter})
		if err != nil {
			return nil, huma.Error500InternalServerError("could not parse capture", err)
		}
		receivedAt := time.Now().UTC()
		directory, err := statement.StoreCapture(root, &capture, result, receivedAt)
		if err != nil {
			return nil, huma.Error500InternalServerError("could not store capture", err)
		}
		return &captureOutput{Body: captureResponse{
			Mode:      result.Mode,
			Directory: directory,
			Capture:   filepath.ToSlash(filepath.Join(directory, "capture.json")),
			Metadata:  filepath.ToSlash(filepath.Join(directory, "problem.json")),
			Statement: filepath.ToSlash(filepath.Join(directory, "statement.md")),
			Warnings:  result.Warnings,
		}}, nil
	})
	huma.Register[struct{}, struct{}](api, huma.Operation{
		OperationID:   "health",
		Method:        http.MethodGet,
		Path:          "/health",
		Summary:       "Check statement server readiness",
		DefaultStatus: http.StatusNoContent,
	}, func(context.Context, *struct{}) (*struct{}, error) { return nil, nil })
}

func serveUntilSignal(server *http.Server) error {
	serverErrors := make(chan error, 1)
	go func() {
		serverErrors <- server.ListenAndServe()
	}()
	signals := make(chan os.Signal, 1)
	signal.Notify(signals, syscall.SIGINT, syscall.SIGTERM)
	defer signal.Stop(signals)
	select {
	case err := <-serverErrors:
		if errors.Is(err, http.ErrServerClosed) {
			return nil
		}
		return err
	case <-signals:
		ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		defer cancel()
		if err := server.Shutdown(ctx); err != nil {
			return err
		}
		return nil
	}
}

func runStatementList(cmd *cobra.Command) error {
	if statementFormat != "table" && statementFormat != "json" && statementFormat != "yaml" {
		return usageError("unsupported list format %q", statementFormat)
	}
	root, err := statement.ResolveRoot(statementOutputDir)
	if err != nil {
		return err
	}
	captures, warnings := statement.Scan(root)
	printInventoryWarnings(cmd, warnings)
	if err := writeProblemRecords(cmd.OutOrStdout(), statement.Summarize(captures), statementFormat); err != nil {
		return err
	}
	if len(warnings) > 0 {
		return errors.New("statement inventory is incomplete")
	}
	return nil
}

func writeProblemRecords(writer io.Writer, records []statement.ProblemRecord, format string) error {
	switch format {
	case "json":
		return json.NewEncoder(writer).Encode(records)
	case "yaml":
		return yaml.NewEncoder(writer).Encode(records)
	default:
		_, err := fmt.Fprintln(writer, "KEY\tTITLE\tTIME\tMEMORY\tCAPTURED")
		if err != nil {
			return err
		}
		for _, record := range records {
			if _, err := fmt.Fprintf(writer, "%s\t%s\t%s\t%s\t%s\n", tableValue(record.Key), tableValue(record.Title), formatLimit(record.TimeMS, "ms"), formatLimit(record.MemoryMiB, "MiB"), record.CapturedAt.Format(time.RFC3339)); err != nil {
				return err
			}
		}
		return nil
	}
}

func formatLimit(value *int64, unit string) string {
	if value == nil {
		return "-"
	}
	return fmt.Sprintf("%d %s", *value, unit)
}
func tableValue(value string) string {
	value = statement.SanitizeTerminalText(value)
	return strings.NewReplacer("\n", " ", "\r", " ", "\t", " ").Replace(value)
}

func runStatementPath(cmd *cobra.Command, identifier string) error {
	if err := validateCaptureSelector(statementCapture); err != nil {
		return err
	}
	if statementFile != "" && statementFile != "capture" && statementFile != "metadata" && statementFile != "statement" {
		return usageError("unsupported file selector %q", statementFile)
	}
	root, err := statement.ResolveRoot(statementOutputDir)
	if err != nil {
		return err
	}
	captures, warnings := statement.Scan(root)
	printInventoryWarnings(cmd, warnings)
	if len(warnings) > 0 {
		return errors.New("statement inventory is incomplete")
	}
	record, err := statement.FindCapture(captures, identifier, statementCapture)
	if err != nil {
		return err
	}
	path := record.Directory
	switch statementFile {
	case "capture":
		path = filepath.ToSlash(filepath.Join(path, "capture.json"))
	case "metadata":
		path = filepath.ToSlash(filepath.Join(path, "problem.json"))
	case "statement":
		path = record.Statement
	}
	_, err = fmt.Fprintln(cmd.OutOrStdout(), path)
	return err
}

func runStatementShow(cmd *cobra.Command, identifier string) error {
	if err := validateCaptureSelector(statementCapture); err != nil {
		return err
	}
	root, err := statement.ResolveRoot(statementOutputDir)
	if err != nil {
		return err
	}
	captures, warnings := statement.Scan(root)
	printInventoryWarnings(cmd, warnings)
	if len(warnings) > 0 {
		return errors.New("statement inventory is incomplete")
	}
	record, err := statement.FindCapture(captures, identifier, statementCapture)
	if err != nil {
		return err
	}
	path := filepath.Join(root, filepath.FromSlash(record.Statement))
	info, err := os.Lstat(path)
	if err != nil {
		return err
	}
	if info.Mode()&os.ModeSymlink != 0 || !info.Mode().IsRegular() {
		return fmt.Errorf("statement artifact is not a regular file: %s", path)
	}
	content, err := os.ReadFile(path)
	if err != nil {
		return err
	}
	stdout := cmd.OutOrStdout()
	if !isTerminalWriter(stdout) {
		_, err = stdout.Write(content)
		return err
	}
	rendered, err := renderStatementMarkdown(string(content), terminalWidth(stdout))
	if err != nil {
		return fmt.Errorf("render statement Markdown: %w", err)
	}
	_, err = io.WriteString(stdout, rendered)
	return err
}

// isTerminalWriter reports whether a destination is interactive. Piped and
// redirected output keeps the raw Markdown artifact instead of the rendered form.
func isTerminalWriter(writer io.Writer) bool {
	file, ok := writer.(*os.File)
	return ok && isatty.IsTerminal(file.Fd())
}

func renderStatementMarkdown(markdown string, width int) (string, error) {
	renderer, err := glamour.NewTermRenderer(
		glamour.WithStylePath(statementStyle()),
		glamour.WithWordWrap(width),
	)
	if err != nil {
		return "", err
	}
	return renderer.Render(markdown)
}

// statementStyle returns the styled terminal theme. An explicit GLAMOUR_STYLE
// wins, the light theme is used only when the terminal advertises a light
// background through COLORFGBG, and the dark theme is the default. Themes are
// chosen without querying the terminal so rendering never waits on a reply.
func statementStyle() string {
	if style := strings.TrimSpace(os.Getenv("GLAMOUR_STYLE")); style != "" {
		return style
	}
	if background, ok := os.LookupEnv("COLORFGBG"); ok {
		parts := strings.Split(background, ";")
		if value, err := strconv.Atoi(strings.TrimSpace(parts[len(parts)-1])); err == nil && value >= 7 {
			return styles.LightStyle
		}
	}
	return styles.DarkStyle
}

// terminalWidth returns the interactive width used for wrapping, defaulting to
// the conventional terminal width when the size is unavailable.
func terminalWidth(writer io.Writer) int {
	file, ok := writer.(*os.File)
	if !ok {
		return defaultStatementWidth
	}
	width, _, err := term.GetSize(int(file.Fd()))
	if err != nil || width <= 0 {
		return defaultStatementWidth
	}
	return width
}

var captureSelectorPattern = regexp.MustCompile(`^[0-9]{8}T[0-9]{6}\.[0-9]{6}Z(?:-[0-9]{2})?$`)

func validateCaptureSelector(value string) error {
	if value != "" && !captureSelectorPattern.MatchString(value) {
		return usageError("invalid capture selector %q", value)
	}
	return nil
}

func printInventoryWarnings(cmd *cobra.Command, warnings []error) {
	for _, warning := range warnings {
		fmt.Fprintf(cmd.ErrOrStderr(), "warning: %v\n", warning)
	}
}

func usageArgs(validator cobra.PositionalArgs) cobra.PositionalArgs {
	return func(cmd *cobra.Command, args []string) error {
		if err := validator(cmd, args); err != nil {
			return &UsageError{Err: err}
		}
		return nil
	}
}

func init() {
	rootCmd.AddCommand(statementCmd)
	statementCmd.PersistentFlags().StringVar(&statementOutputDir, "output-dir", "", "Statement storage root override")
	statementCmd.AddCommand(serveStatementCmd, statementRootCmd, statementListCmd, statementPathCmd, statementShowCmd)
	serveStatementCmd.Flags().StringVar(&statementAdapter, "adapter", "", "Force one statement adapter")
	serveStatementCmd.Flags().IntVar(&statementPort, "port", 27121, "Loopback server port")
	statementListCmd.Flags().StringVar(&statementFormat, "format", "table", "Output format: table, json, or yaml")
	statementPathCmd.Flags().StringVar(&statementCapture, "capture", "", "Exact capture timestamp directory")
	statementPathCmd.Flags().StringVar(&statementFile, "file", "", "Artifact: capture, metadata, or statement")
	statementShowCmd.Flags().StringVar(&statementCapture, "capture", "", "Exact capture timestamp directory")
	rootCmd.SetFlagErrorFunc(func(_ *cobra.Command, err error) error { return &UsageError{Err: err} })
}
