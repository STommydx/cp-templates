# Writing conventions

These rules apply to comments, Markdown, commit messages, and pull request descriptions in the templates repository. The reader is a maintainer who knows the code and domain but did not participate in the change. Write the contract and the reason that explains it. Keep change history in commits and pull requests.

## Prose rules

### Keep the reason

Keep the reason for a rule when editing prose. If a maintainer cannot reconstruct why a constraint exists, the edit removed useful information.

### Describe the present state

Describe what the code does and which constraints hold. Do not describe an earlier implementation, a review thread, or a planned change in durable documentation.

### Use a plain declarative voice

Use direct, literal sentences. Avoid rhetorical questions, exclamation marks, metaphors, idioms, intensifiers, and process labels. Prefer one idea per sentence. Put code-related names, paths, flags, and values in code font.

### Keep documents reviewable

Use sentence-case headings. Use numbered lists for sequences, bullets for parallel items, and tables for paired values. Split long sections by responsibility. Keep package READMEs focused on purpose, usage, invariants, and pitfalls.

### Cite durable sources

Link to package READMEs or documents that exist in the repository. Do not cite temporary plans, session paths, reviewer labels, or unresolved local files from source or durable documentation.

## Comments and doc comments

- Add comments only for non-obvious invariants, constraints, ordering requirements, or workarounds.
- A Go doc comment starts with the identifier and states its contract.
- A body comment explains why a surprising branch or value is required. It does not repeat the next line.
- A test comment names the behavior under test and the failure it would catch.
- Keep long design explanations in the package README rather than in source comments.

## Source contexts

The templates repository contains repository tooling rather than contest submissions. Its C++, Go, JavaScript, and Markdown source uses technical comments and doc comments for non-trivial contracts, invariants, constraints, and platform or library workarounds. Put longer rationale and usage instructions in the relevant package README.


## Commit messages and pull requests

Commit subjects use an imperative verb, a present-tense description, and an area prefix such as `docs(ccli):`. Keep the subject concise. The body states what changed, why the constraint matters, and how the change was verified.

A pull request description uses four short sections:

1. Problem
2. Change
3. Verification
4. Deliberate exclusions

Each section describes the current change. It does not reproduce a session transcript.

## Review checklist

Before committing prose, check:

- Does the text describe the present behavior?
- Does each important constraint include its reason?
- Does every cited path exist in the repository?
- Are code names, flags, and values formatted as code?
- Could a maintainer scan the section without reconstructing conversation history?
