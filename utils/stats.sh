#!/bin/env bash
# emit some stats on all the files in the repository. Exclude file i did not write
git ls-tree -r master --name-only | tr '\n' '\0' | wc --files0-from=- | tr ' ' '\t'