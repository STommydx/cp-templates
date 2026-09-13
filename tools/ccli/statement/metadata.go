package statement

import "time"

// MetadataSchemaVersion identifies the durable problem.json schema.
// ParserVersion identifies the parser provenance written to each capture.
// Both values change only when their persisted contracts change.
const (
	MetadataSchemaVersion = 1
	ParserVersion         = "ccli-statement/1"
)

// ProblemMetadata is the durable normalized record for one capture.
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

// SourceMetadata records parser provenance and browser/local timestamps.
type SourceMetadata struct {
	Adapter       string    `json:"adapter"`
	ParserVersion string    `json:"parser_version"`
	URL           string    `json:"url"`
	CapturedAt    time.Time `json:"captured_at"`
	ReceivedAt    time.Time `json:"received_at"`
}

// ProblemIdentity contains source identity fields used by inventory and storage.
type ProblemIdentity struct {
	Code     string `json:"code,omitempty"`
	Title    string `json:"title"`
	Slug     string `json:"slug"`
	Category string `json:"category,omitempty"`
	Group    string `json:"group,omitempty"`
}

// ProblemLimits stores canonical limits and their displayed source text.
type ProblemLimits struct {
	TimeMS    *int64 `json:"time_ms,omitempty"`
	MemoryMiB *int64 `json:"memory_mib,omitempty"`
	TimeRaw   string `json:"time_raw,omitempty"`
	MemoryRaw string `json:"memory_raw,omitempty"`
}

// ExecutionMetadata stores execution properties only when the source states them.
type ExecutionMetadata struct {
	Interactive *bool  `json:"interactive,omitempty"`
	TestType    string `json:"test_type,omitempty"`
	InputMode   string `json:"input_mode,omitempty"`
	OutputMode  string `json:"output_mode,omitempty"`
	InputFile   string `json:"input_file,omitempty"`
	OutputFile  string `json:"output_file,omitempty"`
}
