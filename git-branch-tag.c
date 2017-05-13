#include <git2/global.h>
#include <git2/diff.h>
#include <git2/status.h>
#include <git2/repository.h>
#include <git2/refs.h>
#include <stdio.h>
#include <string.h>


#define ANSI_BOLD_RED "\033[1;31m"
#define ANSI_BOLD_GREEN "\033[1;32m"
#define ANSI_BOLD_CYAN "\033[1;36m"
#define ANSI_RESET "\033[0m"


int staged, modified, conflict, unknown;

int
update_status(const char *path, unsigned int flags, void *payload) {
    staged |= !!(flags & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED
                | GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED
                | GIT_STATUS_INDEX_TYPECHANGE));
    modified |= !!(flags & (GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED
            | GIT_STATUS_WT_TYPECHANGE | GIT_STATUS_WT_RENAMED));
    conflict |= !!(flags & GIT_STATUS_CONFLICTED);
    unknown |= !!(flags & GIT_STATUS_WT_NEW);

    return 0;
}


int
main(void) {
    git_libgit2_init();

    git_repository *repo;
    if (git_repository_open_ext(&repo, ".", 0, NULL) != 0) return 1;

    git_reference *ref;
    if (git_reference_lookup(&ref, repo, "HEAD") != 0) return 1;

    const char *name = git_reference_symbolic_target(ref);
    if (name) {
        static const char *prefix = "refs/heads/";
        size_t prefix_length = strlen(prefix);

        if (strncmp(name, prefix, prefix_length) == 0) {
            name += prefix_length;
        }
        printf(ANSI_BOLD_CYAN "[%s", name);
    } else {
        const git_oid *oid = git_reference_target(ref);
        if (!oid) return 1;

        char hash[GIT_OID_HEXSZ+1];
        git_oid_tostr(hash, GIT_OID_HEXSZ+1, oid);
        hash[8] = 0;

        printf(ANSI_BOLD_CYAN "(%s", hash);
    }

    git_status_foreach(repo, update_status, NULL);

    if (staged || modified || conflict || unknown) printf(" ");
    if (staged) printf(ANSI_BOLD_GREEN "S");
    if (modified) printf(ANSI_BOLD_RED "M");
    if (conflict) printf(ANSI_BOLD_RED "C");
    if (unknown) printf(ANSI_BOLD_RED "?");

    printf(ANSI_BOLD_CYAN "%s" ANSI_RESET, name ? "]" : ")");

    return 0;
}
