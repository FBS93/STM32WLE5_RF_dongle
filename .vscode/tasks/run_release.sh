#!/usr/bin/env bash
set -euo pipefail

echo "🚀 Run release"

# Parse command-line arguments.
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 [WORKSPACE_ROOT] [TITLE] [VERSION]"
    exit 1
fi

workspace_root="${1%/}"
title="$2"
release_version="$3"

# Validate required inputs.
if [ ! -d "${workspace_root}" ]; then
    echo "❌ Error: workspace directory not found: ${workspace_root}"
    exit 1
fi

if [[ ! "${release_version}" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "❌ Error: release version must match vX.Y.Z."
    exit 1
fi

# Validate required environment variables.
if [ -z "${DOCGEN_OUTPUT-}" ]; then
    echo "❌ Error: DOCGEN_OUTPUT must define the documentation output directory."
    exit 1
fi

if [ -z "${RELEASE_REMOTE-}" ]; then
    echo "❌ Error: RELEASE_REMOTE must define the Git remote for release tags."
    exit 1
fi

# Validate required commands.
for command in bash git python3; do
    if ! command -v "${command}" >/dev/null 2>&1; then
        echo "❌ Error: ${command} not found."
        exit 1
    fi
done

cd "${workspace_root}"

if [ -n "$(git status --porcelain --untracked-files=all)" ]; then
    echo "❌ Error: working tree must be clean before creating a release."
    exit 1
fi

if git show-ref --verify --quiet "refs/tags/${release_version}"; then
    echo "❌ Error: local tag already exists: ${release_version}"
    exit 1
fi

if ! release_remote_url="$(git remote get-url "${RELEASE_REMOTE}")"; then
    echo "❌ Error: unable to resolve RELEASE_REMOTE '${RELEASE_REMOTE}' to a Git URL." >&2
    exit 1
fi

if remote_tag_refs="$(git ls-remote --exit-code --tags "${RELEASE_REMOTE}" "refs/tags/${release_version}" 2>&1)"; then
    echo "❌ Error: remote tag already exists on ${RELEASE_REMOTE}: ${release_version}"
    exit 1
else
    remote_tag_status=$?
    if [ "${remote_tag_status}" -ne 2 ]; then
        echo "❌ Error: unable to check tag on ${RELEASE_REMOTE}: ${remote_tag_refs}" >&2
        exit "${remote_tag_status}"
    fi
fi

# Validate before creating the release tag, avoiding tag rollback for failures
# detected before publication begins.
./.vscode/tasks/validate_release.sh "${workspace_root}" "${title}" \
    "${release_version}"

tag_created=0
tag_pushed=0

echo "Creating and pushing release tag ${release_version}..."
git tag -a "${release_version}" -m "Release ${release_version}"
tag_created=1
if git push "${RELEASE_REMOTE}" "refs/tags/${release_version}"; then
    tag_pushed=1
else
    status=$?
    echo "❌ Release tag push failed with status ${status}; removing ${release_version}." >&2
    if [ "${tag_created}" -eq 1 ]; then
        git tag -d "${release_version}" || \
            echo "❌ Error: could not remove local tag ${release_version}." >&2
    fi
    exit "${status}"
fi

# Publish from an isolated checkout so generated documentation never changes
# the release working tree; the EXIT trap removes it on every exit path.
if pages_root="$(mktemp -d)" &&
    pages_dir="${pages_root}/gh-pages" &&
    trap 'rm -rf "${pages_root}"' EXIT &&
    { echo "Publishing documentation to ${RELEASE_REMOTE}/gh-pages..."; } &&
    {
        if pages_refs="$(git ls-remote --exit-code --heads "${RELEASE_REMOTE}" \
            "refs/heads/gh-pages" 2>&1)"; then
            git clone --branch gh-pages --single-branch "${release_remote_url}" "${pages_dir}"
        else
            pages_status=$?
            if [ "${pages_status}" -ne 2 ]; then
                echo "❌ Error: unable to check gh-pages on ${RELEASE_REMOTE}: ${pages_refs}" >&2
                (exit "${pages_status}")
            else
                # A missing remote branch is the first publication: create an
                # orphan branch instead of requiring gh-pages to exist first.
                echo "Creating the first gh-pages publication branch..."
                git clone "${release_remote_url}" "${pages_dir}" &&
                    git -C "${pages_dir}" checkout --orphan gh-pages &&
                    { git -C "${pages_dir}" rm -rf . || :; }
            fi
        fi
    } &&
    # Keep each release's generated site so the publication index can link to
    # all released documentation versions.
    version_dir="${pages_dir}/${release_version}" &&
    rm -rf "${version_dir}" &&
    mkdir -p "${version_dir}" &&
    cp -a "${DOCGEN_OUTPUT}/." "${version_dir}/" &&
    : > "${pages_dir}/.nojekyll" &&
    {
        printf '%s\n' '<!doctype html>'
        printf '%s\n' '<html lang="en">' '<head>'
        printf '%s\n' '  <meta charset="utf-8">'
        printf '%s\n' '  <meta name="viewport" content="width=device-width, initial-scale=1">'
        printf '%s\n' '  <title>Release documentation</title>'
        printf '%s\n' '</head>' '<body>'
        printf '%s\n' '  <h1>Release documentation</h1>' '  <ul>'
        while IFS= read -r version; do
            printf '    <li><a href="%s/">%s</a></li>\n' "${version}" "${version}"
        done < <(
            for version_dir in "${pages_dir}"/v*; do
                version="${version_dir##*/}"
                if [ -d "${version_dir}" ] && \
                    [[ "${version}" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
                    printf '%s\n' "${version}"
                fi
            done | sort -V
        )
        printf '%s\n' '  </ul>' '</body>' '</html>'
    } > "${pages_dir}/index.html" &&
    git -C "${pages_dir}" add --all &&
    git -C "${pages_dir}" commit -m "Publish documentation for ${release_version}" &&
    git -C "${pages_dir}" push origin gh-pages; then
    :
else
    status=$?
    # The release tag identifies published documentation, so remove both local
    # and remote tags if documentation publication cannot complete.
    echo "❌ Documentation publication failed with status ${status}; removing ${release_version}." >&2
    if [ "${tag_pushed}" -eq 1 ]; then
        git push "${RELEASE_REMOTE}" ":refs/tags/${release_version}" || \
            echo "❌ Error: could not remove remote tag ${release_version}." >&2
    fi
    if [ "${tag_created}" -eq 1 ]; then
        git tag -d "${release_version}" || \
            echo "❌ Error: could not remove local tag ${release_version}." >&2
    fi
    exit "${status}"
fi

echo "✅ Release ${release_version} completed."
