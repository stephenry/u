##========================================================================== //
## Copyright (c) 2025, Stephen Henry
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of source code must retain the above copyright notice, this
##   list of conditions and the following disclaimer.
##
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation
##   and/or other materials provided with the distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##========================================================================== //

import os
import pathlib

class SynthesisFlow:
    def __init__(self, worklist: dict, **kwargs):
        self._worklist = worklist
        self._top_level_template = kwargs.get('top', None)

    def run(self) -> int:
        for f in self._frequencies():
            self._run(f)

    def _frequencies(self):
        if 'frequency' in self._worklist:
            yield int(self._worklist['frequency'])
        elif 'sweep' in self._worklist:
            lo, hi, inc = self._worklist['sweep'].split(',')
            lo_f = int(lo)
            hi_f = int(hi)
            inc_f = int(inc)
            f = lo_f
            while f <= hi_f:
                yield f
                f += inc_f
        else:
            raise RuntimeError('No frequency or sweep specified.')

    def _run(self, freq: int) -> int:
        run_root = pathlib.Path(f'{self._worklist["name"]}_{freq}MHz')
        run_root.mkdir(exist_ok=True)

        top_sv_path = self._render_toplevel(run_root)

        srcs = self._worklist['sources'] + [str(top_sv_path.resolve())]

        # Run synlig
        self._render_synlig_script(run_root, freq, srcs)
        if self._run_synlig() != 0:
            pass

        # Run OpenSTA
        self._render_opensta_script(run_root, freq)
        if self._run_opensta() != 0:
            pass

    def _render_toplevel(self, run_root: pathlib.Path) -> pathlib.Path:
        import jinja2

        t = jinja2.Template(self._top_level_template.read_text())

        def to_verilog_parameter(b: bool) -> str:
            return '1\'b1' if b else '1\'b0'

        env = {
            'W': 32,
            'ADMIT_COMPLIMENT_EN': to_verilog_parameter(True),
            'unary_detector': self._worklist['name'],
        }

        top_level_out = run_root / 'top.sv'
        top_level_out.write_text(t.render(env))
        return top_level_out

    def _render_synlig_script(self, run_root: pathlib.Path, freq: int, srcs: list[str] = []):
        with open(run_root / 'synlig.tcl', 'w') as f:
            f.write(f'# Synlig script\n')
            f.write(f'# Project: {self._worklist["name"]}\n')
            f.write(f'# Frequency: {freq} MHz\n')

            include_paths = [f'-I{inc}' for inc in self._worklist.get('include_paths', [])]

            cmds = list()
            for src in srcs:
                cmds.append(f'read_systemverilog {" ".join(include_paths)} -defer {src}')

            cmds += [
                'read_systemverilog -link',
                'hierarchy -top top',
                'flatten',
                'proc',
                'opt',
                'techmap',
                'opt',
                'write_verilog syn.v'
            ]
            f.write('\n'.join(cmds) + '\n')

    def _render_opensta_script(self, run_root: pathlib.Path, freq: int):
        with open(run_root / 'opensta.tcl', 'w') as f:
            f.write(f'# OpenSTA script\n')
            f.write(f'# Project: {self._worklist["name"]}\n')
            f.write(f'# Frequency: {freq} MHz\n')
            # Additional script content would go here

    def _run_synlig(self) -> int:
        pass

    def _run_opensta(self) -> int:
        pass



def run_synthesis_flow(wls: list[dict], **kwargs) -> int:
    results = []
    for wl in wls:
        flow = SynthesisFlow(wl, **kwargs)
        result = flow.run()
        results.append(result)

    return 0
