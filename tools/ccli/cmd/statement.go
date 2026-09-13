package cmd

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"log/slog"
	"net/http"
	"os"
	"os/signal"
	"path/filepath"
	"regexp"
	"strings"
	"syscall"
	"time"

	"charm.land/glamour/v2"
	"charm.land/log/v2"
	"github.com/STommydx/cp-templates/tools/ccli/statement"
	"github.com/STommydx/cp-templates/tools/ccli/statement/adapters/hkoi"
	huma "github.com/danielgtaylor/huma/v2"
	"github.com/danielgtaylor/huma/v2/adapters/humachi"
	"github.com/go-chi/chi/v5"
	"github.com/go-chi/httplog/v3"
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
	statementLogLevel  string
	statementLogFormat string
)

// defaultStatementWidth is the wrapping width when the terminal size is unknown.
const defaultStatementWidth = 80

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
	level, err := log.ParseLevel(statementLogLevel)
	if err != nil {
		return usageError("%v", err)
	}
	formatter, err := statementLogFormatter()
	if err != nil {
		return usageError("%v", err)
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

	logger := log.NewWithOptions(cmd.ErrOrStderr(), log.Options{
		Level:           level,
		Formatter:       formatter,
		ReportTimestamp: true,
	})
	router := chi.NewRouter()
	router.Use(httplog.RequestLogger(slog.New(logger), &httplog.Options{Level: requestLogLevel(logger)}))
	api := humachi.New(router, huma.DefaultConfig("ccli statement server", "1.0.0"))
	registerStatementAPI(api, root, adapters, statementAdapter, logger)
	server := &http.Server{
		Addr:              fmt.Sprintf("127.0.0.1:%d", statementPort),
		Handler:           router,
		ReadHeaderTimeout: 5 * time.Second,
		ReadTimeout:       30 * time.Second,
		WriteTimeout:      30 * time.Second,
		IdleTimeout:       60 * time.Second,
		MaxHeaderBytes:    64 << 10,
	}
	logger.Info("statement server listening", "addr", fmt.Sprintf("http://127.0.0.1:%d", statementPort))
	logger.Info("statement storage", "root", root)
	if err := serveUntilSignal(server); err != nil {
		return err
	}
	logger.Info("statement server stopped")
	return nil
}

// statementLogFormatter maps the log format flag onto a charm log formatter.
func statementLogFormatter() (log.Formatter, error) {
	switch statementLogFormat {
	case "", "text":
		return log.TextFormatter, nil
	case "json":
		return log.JSONFormatter, nil
	default:
		return 0, fmt.Errorf("unsupported log format %q", statementLogFormat)
	}
}

// requestLogLevel reports the lowest response level the request logger records.
// Debug records every request; coarser levels record failures only.
func requestLogLevel(logger *log.Logger) slog.Level {
	if logger.GetLevel() == log.DebugLevel {
		return slog.LevelDebug
	}
	return slog.LevelWarn
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

func registerStatementAPI(api huma.API, root string, adapters []statement.Adapter, forcedAdapter string, logger *log.Logger) {
	huma.Register[captureInput, captureOutput](api, huma.Operation{
		OperationID:   "capture-statement-html",
		Method:        http.MethodPost,
		Path:          "/",
		Summary:       "Capture rendered PageMole statement HTML",
		Description:   "Accept a PageMole statement-html version 1 envelope and persist immutable raw, metadata, and Markdown artifacts. Captures are JSON: the generated octet-stream request body describes the raw-byte capture field and is rejected with 415.",
		DefaultStatus: http.StatusCreated,
		MaxBodyBytes:  statement.MaxCaptureBytes + 1,
		Errors:        []int{http.StatusBadRequest, http.StatusRequestEntityTooLarge, http.StatusUnsupportedMediaType, http.StatusUnprocessableEntity, http.StatusInternalServerError},
	}, func(ctx context.Context, input *captureInput) (*captureOutput, error) {
		capture := input.Body.WithRawBytes(input.RawBody)
		result, err := statement.ParseCapture(&capture, adapters, statement.ParseOptions{ForcedAdapterID: forcedAdapter})
		if err != nil {
			logger.Error("capture parse failed", "url", capture.URL, "err", err)
			return nil, huma.Error500InternalServerError("could not parse capture", err)
		}
		receivedAt := time.Now().UTC()
		directory, err := statement.StoreCapture(root, &capture, result, receivedAt)
		if err != nil {
			logger.Error("capture store failed", "url", capture.URL, "err", err)
			return nil, huma.Error500InternalServerError("could not store capture", err)
		}
		logger.Info("capture stored", "key", directory, "mode", result.Mode, "samples", len(result.Metadata.Samples))
		for _, warning := range result.Warnings {
			logger.Warn("parser warning", "key", directory, "warning", warning)
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
	signals, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()
	serverErrors := make(chan error, 1)
	go func() {
		serverErrors <- server.ListenAndServe()
	}()
	select {
	case err := <-serverErrors:
		if errors.Is(err, http.ErrServerClosed) {
			return nil
		}
		return err
	case <-signals.Done():
		ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		defer cancel()
		return server.Shutdown(ctx)
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
		glamour.WithEnvironmentConfig(),
		glamour.WithWordWrap(width),
	)
	if err != nil {
		return "", err
	}
	return renderer.Render(markdown)
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
	serveStatementCmd.Flags().StringVar(&statementLogLevel, "log-level", "info", "Log level: debug, info, warn, or error")
	serveStatementCmd.Flags().StringVar(&statementLogFormat, "log-format", "text", "Log format: text or json")
	statementListCmd.Flags().StringVar(&statementFormat, "format", "table", "Output format: table, json, or yaml")
	statementPathCmd.Flags().StringVar(&statementCapture, "capture", "", "Exact capture timestamp directory")
	statementPathCmd.Flags().StringVar(&statementFile, "file", "", "Artifact: capture, metadata, or statement")
	statementShowCmd.Flags().StringVar(&statementCapture, "capture", "", "Exact capture timestamp directory")
	rootCmd.SetFlagErrorFunc(func(_ *cobra.Command, err error) error { return &UsageError{Err: err} })
}
