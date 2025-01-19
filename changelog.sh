#!/bin/sh

# Remove some legacy tags that aren't strictly releases
git tag -d MATLAB_API_v0.0
git tag -d Test_Integrate_3.20.1
git tag -d Test_Integrate_3.20.1a


# Extract tags, sorted by date, formatting as markdown file with link to release.

git for-each-ref --sort=-creatordate refs/tags \
--format='# [%(contents:subject)](https://github.com/OpenVSP/OpenVSP/releases/tag/%(refname:short))

%(committerdate:short)

%(contents:body)

---

'
