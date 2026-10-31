# ArcTrack Telemetry Systems

Hardware and firmware repository for ArcTrack telemetry products.

## Structure

```
gen6/
  hardware/
    tracker/        — PCB, schematics, BOM, Gerbers for the tracker unit
    base-station/   — PCB, schematics, BOM, Gerbers for the base station
    _shared/        — KiCad symbols, footprints, 3D models shared across boards
  firmware/
    tracker/
      common/       — shared drivers and HAL for the tracker
      main/         — tracker application entry point and config
    base-station/
      common/       — shared drivers and HAL for the base station
      main/         — base station application entry point and config
    _shared/        — protocol implementations, utilities used by both devices
  docs/
    hardware/       — design notes, errata, component selection rationale
    firmware/       — architecture notes, protocol specs
    system/         — end-to-end system diagrams and integration notes

tools/              — flash scripts, BOM export helpers, test-jig utilities
```

## Git LFS

Large binary files (KiCad PCB/schematic, Altium, STEP models) should be
tracked with Git LFS. Run once after cloning:

```
git lfs install
git lfs track "*.kicad_pcb" "*.kicad_sch" "*.PcbDoc" "*.SchDoc" "*.step" "*.stl"
git add .gitattributes
```

## Generations

This repository covers **Gen 6** onwards. Earlier generations are maintained
as separate legacy projects.
