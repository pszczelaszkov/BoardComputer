#!/usr/bin/env bash
# Parse avr-size output, optionally compare to a previous size.json, write summary + artifact.
set -euo pipefail

ELF="${1:?ELF path required}"
PREV_JSON="${2:-}"
OUT_JSON="${3:-size.json}"
SUMMARY="${GITHUB_STEP_SUMMARY:-}"

SIZE_OUTPUT=$(avr-size --mcu=atmega324pa -C "${ELF}")

PROGRAM_BYTES=$(echo "${SIZE_OUTPUT}" | sed -n 's/^Program:[[:space:]]*\([0-9]*\).*/\1/p')
DATA_BYTES=$(echo "${SIZE_OUTPUT}" | sed -n 's/^Data:[[:space:]]*\([0-9]*\).*/\1/p')
PROGRAM_PCT=$(echo "${SIZE_OUTPUT}" | sed -n 's/^Program:.*(\([0-9.]*\)%.*/\1/p')
DATA_PCT=$(echo "${SIZE_OUTPUT}" | sed -n 's/^Data:.*(\([0-9.]*\)%.*/\1/p')

if [[ -z "${PROGRAM_BYTES}" || -z "${DATA_BYTES}" ]]; then
  echo "error: failed to parse avr-size output:" >&2
  echo "${SIZE_OUTPUT}" >&2
  exit 1
fi

SHA=$(git rev-parse HEAD)

cat > "${OUT_JSON}" <<EOF
{
  "sha": "${SHA}",
  "program_bytes": ${PROGRAM_BYTES},
  "data_bytes": ${DATA_BYTES},
  "program_pct": ${PROGRAM_PCT},
  "data_pct": ${DATA_PCT}
}
EOF

format_delta() {
  local current=$1
  local previous=$2
  if [[ -z "${previous}" ]]; then
    echo "n/a"
    return
  fi
  local delta=$((current - previous))
  if (( delta > 0 )); then
    printf "+%d" "${delta}"
  elif (( delta < 0 )); then
    printf "%d" "${delta}"
  else
    echo "0"
  fi
}

PREV_PROGRAM=""
PREV_DATA=""
if [[ -n "${PREV_JSON}" && -f "${PREV_JSON}" ]]; then
  PREV_PROGRAM=$(python3 -c "import json; print(json.load(open('${PREV_JSON}'))['program_bytes'])")
  PREV_DATA=$(python3 -c "import json; print(json.load(open('${PREV_JSON}'))['data_bytes'])")
fi

PROG_DELTA=$(format_delta "${PROGRAM_BYTES}" "${PREV_PROGRAM}")
DATA_DELTA=$(format_delta "${DATA_BYTES}" "${PREV_DATA}")

if [[ -n "${SUMMARY}" ]]; then
  {
    echo "## AVR HW1 firmware size"
    echo ""
    echo '```'
    echo "${SIZE_OUTPUT}"
    echo '```'
    echo ""
    echo "| Metric | Bytes | % Full | Δ vs previous master |"
    echo "| --- | ---: | ---: | ---: |"
    echo "| Program | ${PROGRAM_BYTES} | ${PROGRAM_PCT}% | ${PROG_DELTA} |"
    echo "| Data | ${DATA_BYTES} | ${DATA_PCT}% | ${DATA_DELTA} |"
    echo ""
    echo "Commit: \`${SHA}\`"
  } >> "${SUMMARY}"
fi

echo "${SIZE_OUTPUT}"
echo "---"
echo "Program: ${PROGRAM_BYTES} bytes (${PROGRAM_PCT}%)  Δ ${PROG_DELTA}"
echo "Data: ${DATA_BYTES} bytes (${DATA_PCT}%)  Δ ${DATA_DELTA}"
echo "Wrote ${OUT_JSON}"
