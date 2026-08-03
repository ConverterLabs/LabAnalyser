# Fixture rules

This directory contains compatibility fixtures, not generated build output.
The directories are intentionally empty in milestone 2A; no golden data has
been inferred or created.

| Directory | Intended contract |
| --- | --- |
| `xml/` | Experiment/project XML read, write, and backward compatibility |
| `plugins/` | Plugin descriptors and load/ABI cases |
| `remote-control/` | Wire bytes and request/response sequences |
| `export/mat/` | libmatio MAT content |
| `export/hdf5/` | HDF5 content |
| `parameters/` | Parameter import/export inputs |
| `gui/` | Deterministic GUI workflow inputs or semantic snapshots |

Fixtures must be small, deterministic, free of secrets and personal data, and
reviewable as text or compact binary data.  Record origin, producer version,
normalization rules, and the consuming test ID beside each fixture.  Preserve
the baseline fixture exactly; do not replace a golden file merely to make a
candidate implementation pass.  Canonicalize only documented nondeterministic
fields (for example timestamps or absolute paths), and compare semantic content
where formatting is not contractual.
