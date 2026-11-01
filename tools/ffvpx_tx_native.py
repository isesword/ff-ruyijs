"""Build the frozen ffvpx av_tx sources (Firefox 159, LGPL-2.1-or-later) as a
standalone object group for the macOS arm64 audio path. The ffvpx sources are
kept separate from the AGPL engine sources and ship as relocatable objects,
the same arrangement Mozilla uses for ffvpx: a recipient can relink or replace
them (see vendor/ffvpx-tx-native/README.md). Only the inverse real DFT used
by blink's PeriodicWave is exercised, through the engine's own declarations —
the libavutil headers never enter the engine's include graph."""
import hashlib
import json
from pathlib import Path


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def frozen_inputs(source):
    manifest = json.loads((source / 'manifest.json').read_text('utf8'))
    for row in manifest['files']:
        actual = digest(source / row['path'])
        if actual != row['sha256']:
            raise ValueError('ffvpx-tx source differs from frozen manifest: '
                             + row['path'])
    return manifest


def build(native):
    if not (native.donor.rust_target.startswith('aarch64-apple-darwin')
            or (hasattr(native.donor, 'rust_target')
                and 'aarch64' in str(native.donor.rust_target)
                and 'darwin' in str(native.donor.rust_target))):
        return []  # other platforms keep the portable emulation
    source = native.root / 'vendor/ffvpx-tx-native'
    directory = native.directory / 'ffvpx-tx-native'
    directory.mkdir(parents=True, exist_ok=True)
    frozen_inputs(source)
    config = source / 'config'
    flags = ['-DHAVE_AV_CONFIG_H', '-I' + str(config), '-I' + str(source),
             '-I' + str(source / 'libavutil')]
    c_units = ['libavutil/tx.c', 'libavutil/tx_float.c', 'libavutil/bprint.c',
               'libavutil/log.c', 'libavutil/mem.c', 'libavutil/error.c',
               'libavutil/cpu.c', 'libavutil/tx_double.c', 'libavutil/tx_int32.c',
               'libavutil/mathematics.c', 'libavutil/time.c',
               'libavutil/aarch64/tx_float_init.c',
               'libavutil/aarch64/cpu.c']
    asm_units = ['libavutil/aarch64/tx_float_neon.S']
    objects = []
    identity = {'sources': digest(source / 'manifest.json'),
                'builder': digest(__file__)}
    stamp = directory / 'build.json'
    signature = hashlib.sha256(
        json.dumps(identity, sort_keys=True).encode()).hexdigest()
    expected = {}
    if stamp.is_file() and json.loads(stamp.read_text('utf8')).get(
            'signature') == signature and not native.incremental['disabled']:
        for unit in c_units + asm_units:
            obj = directory / (Path(unit).stem + '.o')
            if obj.is_file():
                objects.append(obj)
                expected[obj.name] = digest(obj)
        if len(objects) == len(c_units) + len(asm_units) and all(
                digest(p) == expected[p.name] for p in objects):
            return objects
    for unit in c_units:
        stem = Path(unit).stem
        if unit.count('/') > 1:
            stem = Path(unit).parent.name + '_' + stem
        obj = directory / (stem + '.o')
        command = native.compile_foreign(source / unit, obj, flags)
        completed = subprocess_run(command)
        if completed != 0:
            raise RuntimeError('ffvpx-tx compile failed: ' + unit)
        objects.append(obj)
    for unit in asm_units:
        stem = Path(unit).stem
        if unit.count('/') > 1:
            stem = Path(unit).parent.name + '_' + stem
        obj = directory / (stem + '.o')
        command = native.compile_foreign(source / unit, obj, flags)
        completed = subprocess_run(command)
        if completed != 0:
            raise RuntimeError('ffvpx-tx assemble failed: ' + unit)
        objects.append(obj)
    stamp.write_text(json.dumps(
        {'signature': signature,
         'outputs': {p.name: digest(p) for p in objects}}, indent=1) + '\n',
        'utf8')
    return objects


def subprocess_run(command):
    import subprocess
    completed = subprocess.run(command, capture_output=True, text=True)
    if completed.returncode:
        raise RuntimeError('ffvpx-tx: ' + (completed.stdout +
                                           completed.stderr)[-4000:])
    return completed.returncode
