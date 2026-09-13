package statement

import "time"

const (
	MetadataSchemaVersion = 1
	ParserVersion         = "ccli-statement/1"
)

type ProblemMetadata struct {
	SchemaVersion int               `json:"schema_version"`
	Mode          string            `json:"mode"`
	Source        SourceMetadata    `json:"source"`
	Identity      ProblemIdentity   `json:"identity"`
	Limits        ProblemLimits     `json:"limits"`
	Execution     ExecutionMetadata `json:"execution"`
	Samples       []Sample          `json:"samples"`
	Warnings      []string          `json:"warnings,omitempty"`
}

type SourceMetadata struct {
	Adapter       string    `json:"adapter"`
	ParserVersion string    `json:"parser_version"`
	URL           string    `json:"url"`
	CapturedAt    time.Time `json:"captured_at"`
	ReceivedAt    time.Time `json:"received_at"`
}

type ProblemIdentity struct {
	Code     string `json:"code,omitempty"`
	Title    string `json:"title"`
	Slug     string `json:"slug"`
	Category string `json:"category,omitempty"`
	Group    string `json:"group,omitempty"`
}

type ProblemLimits struct {
	TimeMS    *int64 `json:"time_ms,omitempty"`
	MemoryMiB *int64 `json:"memory_mib,omitempty"`
	TimeRaw   string `json:"time_raw,omitempty"`
	MemoryRaw string `json:"memory_raw,omitempty"`
}

type ExecutionMetadata struct {
	Interactive *bool  `json:"interactive,omitempty"`
	TestType    string `json:"test_type,omitempty"`
	InputMode   string `json:"input_mode,omitempty"`
	OutputMode  string `json:"output_mode,omitempty"`
	InputFile   string `json:"input_file,omitempty"`
	OutputFile  string `json:"output_file,omitempty"`
}
