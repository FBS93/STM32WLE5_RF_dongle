# third_party_sync overview

`third_party_sync.py` synchronizes third-party snapshots into workspace paths declared in `repos.json` and tracks resolved snapshot state in `repos.lock.json`.

# Glossary

| Term | Definition |
|---|---|
| Manifest | Editable configuration file (`repos.json`) containing repository entries (`id`, `url`, `path`, `ref_type`, `ref`). |
| Lock file | Generated file (`repos.lock.json`) storing resolved snapshot metadata for each repository `id`. |
| Pinned reference | A non-empty `ref` value interpreted according to `ref_type` and resolved to one exact release tag or commit SHA. |

# Usage example

Options:
- `--repository-id <id>`: Synchronize only the manifest entry matching `id`.
- `--manifest <path>`: Path to manifest file.
- `--lock <path>`: Path to lock file.
- `--workspace <path>`: Workspace root used to resolve repository paths.
- `--dry-run`: Show updates without writing files.

Environment:
- `GH_TOKEN`: Optional personal access token for GitHub API requests.

```bash
python tools/third_party_sync/third_party_sync.py
```

Synchronize only one manifest entry by `id`:

```bash
python tools/third_party_sync/third_party_sync.py --repository-id repo_a
```

Manifest format (`repos.json`):

```json
{
  "repositories": [
    {
      "id": "repo_a",
      "url": "https://github.com/<owner>/<repo_a>",
      "path": "third_party/<domain>/<repo_a>",
      "ref_type": "auto",
      "ref": ""
    },
    {
      "id": "repo_b",
      "url": "https://github.com/<owner>/<repo_b>",
      "path": "third_party/<domain>/<repo_b>",
      "ref_type": "release",
      "ref": "v1.0.0"
    }
  ]
}
```

Lock format (`repos.lock.json`):

```json
{
  "repositories": {
    "repo_a": {
      "source": "release",
      "resolved_release": "v1.18.2",
      "resolved_commit": "0123456789abcdef0123456789abcdef01234567",
      "published_at": "2025-08-01T12:00:00Z",
      "tarball_url": "https://api.github.com/repos/<owner>/<repo_a>/tarball/v1.18.2"
    },
    "repo_b": {
      "source": "default_branch_head",
      "default_branch": "main",
      "resolved_commit": "89abcdef0123456789abcdef0123456789abcdef",
      "commit_date": "2024-05-10T09:30:00Z",
      "tarball_url": "https://api.github.com/repos/<owner>/<repo_b>/tarball/89abcdef0123456789abcdef0123456789abcdef"
    }
  }
}
```

Behavior rules:

- If `ref_type` is `auto` and `ref` is `""`, the script resolves latest release and falls back to default branch HEAD when no release exists.
- If `ref_type` is `release` and `ref` is `""`, the script resolves latest release only.
- If `ref_type` is `release` and `ref` is non-empty, the script pins to that exact release tag.
- If `ref_type` is `commit` and `ref` is `""`, the script uses default branch HEAD.
- If `ref_type` is `commit` and `ref` is non-empty, the script pins to that exact commit SHA.
- Invalid `ref_type`, missing release tag, or missing commit SHA produce explicit errors and stop execution.
- If a manifest entry is removed or its `id` changes, previously downloaded content is not removed.
