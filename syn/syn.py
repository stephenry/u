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

class SynligRunner:
    def __init__(self, **kwargs):
        self._path = kwargs.get('path')
        self._project = kwargs.get('project')
        self._w = kwargs.get('w')
        self._frequency = kwargs.get('frequency')
        self._sources = kwargs.get('sources', [])
        self._include_paths = kwargs.get('include_paths', [])
        self._syn_v = kwargs.get('syn_v', 'syn.v')
        self._script_tcl = kwargs.get('script_tcl', 'synlig.tcl')
        self._top_level_template = kwargs.get('top_level_template')
        self._top_sv = kwargs.get('top_sv', 'top.sv')
        self._echo =  kwargs.get('echo', False)
        self._total_area = None
        self._sequential_area = None

    def run(self):
        self._render_toplevel()
        self._render_synlig_script()
        ec, stdout = self._run_synlig()
        if self._echo:
            print(stdout)
        self._total_area, self._sequential_area = self._scan_synlig_output(stdout)

    def area(self) -> tuple[float, float]:
        return (self._total_area, self._sequential_area)

    def _render_toplevel(self):
        import jinja2

        tl = pathlib.Path(self._top_level_template)
        t = jinja2.Template(tl.read_text())

        def to_verilog_parameter(b: bool) -> str:
            return '1\'b1' if b else '1\'b0'

        env = {
            'W': self._w,
            'ADMIT_COMPLIMENT_EN': to_verilog_parameter(True),
            'unary_detector': self._project,
        }

        top_level_out = self._path / self._top_sv
        top_level_out.write_text(t.render(env))
        self._sources.append(str(top_level_out.resolve()))

    def _render_synlig_script(self):
        from cfg import STDCELL_LIB_PATH

        with open(self._path / self._script_tcl, 'w') as f:
            f.write(f'# Synlig script\n')
            f.write(f'# Project: {self._project}\n')
            f.write(f'# Frequency: {self._frequency} MHz\n')

            cmds = []
 
            include_files = [
                f'-I{include_path}' for include_path in self._include_paths]

            for src in self._sources:
                cmds.append(f'read_systemverilog {" ".join(include_files)} -defer {src}')

            cmds += [
                'read_systemverilog -link',
                'hierarchy -check -top top',
                'flatten',
                'proc',
                'opt',
                'dfflegalize',
                'techmap',
                f'dfflibmap -liberty {STDCELL_LIB_PATH}',
                f'abc -liberty {STDCELL_LIB_PATH}',
                'opt',
                'opt_clean -purge',
                'check',
                f'write_verilog -noattr -noexpr {self._syn_v}',
                f'stat -liberty {STDCELL_LIB_PATH}',
            ]
            f.write('\n'.join(cmds) + '\n')

    def _run_synlig(self) -> int:
        from cfg import SYNLIG_EXECUTABLE

        from subprocess import Popen, PIPE
        p = Popen([SYNLIG_EXECUTABLE, '-s', self._script_tcl],
            stdout=PIPE, stderr=PIPE, cwd=self._path)
        output, err = p.communicate()
        return (p.returncode, output.decode())

    def _scan_synlig_output(self, stdout: str):
        import re

        total_area = None
        sequential_area = None

        for line in stdout.splitlines():
            if m := re.search(r'Chip area for module \'\\top\': ([\d\.]+)', line):
                total_area = float(m.group(1))
            elif m := re.search(r'of which used for sequential elements: ([\d\.]+)', line):
                sequential_area = float(m.group(1))

        return (total_area, sequential_area)

class OpenSTARunner:
    def __init__(self, **kwargs):
        self._top = kwargs.get('top', 'top')
        self._syn_v = kwargs.get('syn_v', 'syn.v')
        self._project = kwargs.get('project')
        self._path = kwargs.get('path')
        self._frequency = kwargs.get('frequency')
        self._sdc_file = 'design.sdc'
        self._opensta_file = 'opensta.tcl'
        self._echo = kwargs.get('echo', False)
        self._passed = False

    def run(self):
        self._render_sdc()
        self._render_opensta_script()
        ec, stdout = self._run_opensta()
        if self._echo:
            print(stdout)
        if ec != 0:
            pass
        self._passed = self._scan_opensta_output(stdout)

    def passed(self) -> bool:
        return self._passed

    def _render_sdc(self):
        with open(self._path / self._sdc_file, 'w') as f:
            f.write(f'# SDC file\n')
            f.write(f'# Project: {self._project}\n')
            f.write(f'# Frequency: {self._frequency} MHz\n')
            period_ns = 1000 / self._frequency
            f.write(f'create_clock -name clk -period {period_ns:.3f} [get_ports clk]\n')

    def _render_opensta_script(self):
        from cfg import STDCELL_LIB_PATH

        with open(self._path / self._opensta_file, 'w') as f:
            f.write(f'# OpenSTA script\n')
            f.write(f'# Project: {self._project}\n')
            f.write(f'# Frequency: {self._frequency} MHz\n')
            cmds = [
                f'read_liberty {STDCELL_LIB_PATH}',
                f'read_verilog {self._syn_v}',
                f'link_design {self._top}',
                f'read_sdc {self._sdc_file}',
                f'report_checks',
            ]
            f.write('\n'.join(cmds) + '\n')


    def _run_opensta(self) -> int:
        from cfg import OPENSTA_EXECUTABLE

        from subprocess import Popen, PIPE
        p = Popen([OPENSTA_EXECUTABLE, self._opensta_file],
            stdout=PIPE, stderr=PIPE, cwd=self._path)
        output, err = p.communicate()
        return p.returncode, output.decode()

    def _scan_opensta_output(self, stdout: str):
        import re

        passed = True
        for line in stdout.splitlines():
            if re.search(r'slack \(VIOLATED\)', line):
                passed = False

        return passed

class InstanceRunner:
    def __init__(self, **kwargs):
        self._project = kwargs.get('project')
        self._frequency = kwargs.get('frequency', [])
        self._w = kwargs.get('w', [])
        self._top_level_template = kwargs.get('top_level_template')
        self._sources = kwargs.get('sources', [])
        self._include_paths = kwargs.get('include_paths', [])
        self._instance_name = f'{self._project}_W{self._w}_{self._frequency}MHz'
        self._instance_path = pathlib.Path(self._instance_name)
        self._instance_path.mkdir(exist_ok=True)
        self._echo = kwargs.get('echo', False)

        self._script_tcl = 'synlig.tcl'
        self._sr = None
        self._or = None

    def run(self):
        print(f'Running instance: {self._instance_name}')

        syn_v = 'syn.v'

        args = {
            'syn_v': syn_v,
            'project': self._project,
            'path': self._instance_path,
            'w': self._w,
            'frequency': self._frequency,
            'sources': self._sources,
            'include_paths': self._include_paths,
            'top_level_template': self._top_level_template,
            'echo': self._echo,
        }
        self._sr = SynligRunner(**args)
        self._sr.run()

        args = {
            'syn_v': syn_v,
            'project': self._project,
            'path': self._instance_path,
            'frequency': self._frequency,
            'echo': self._echo,
        }
        self._or = OpenSTARunner(**args)
        self._or.run()

    def results(self) -> dict:
        class Result:
            def __init__(self, sr: SynligRunner, or_: OpenSTARunner):
                self._sr = sr
                self._or = or_

            def total_area(self) -> float:
                return self._sr.area()[0]

            def sequential_area(self) -> float:
                return self._sr.area()[1]

            def timing_passed(self) -> bool:
                return self._or.passed()

            def __str__(self) -> str:
                ta, sa = self._sr.area()
                tp = self._or.passed()
                return (f'Total Area: {ta:0.2f}, '
                        f'Sequential Area: {sa:0.2f}, Combinatorial Area: {ta - sa:0.2f} '
                        f'Timing Passed: {tp}')

        return Result(self._sr, self._or)

class ProjectRunner:
    def __init__(self, **kwargs):
        self._project = kwargs.get('project')
        self._frequency_sweep = kwargs.get('frequency_sweep', [])
        self._top_level_template = kwargs.get('top_level_template')
        self._w_sweep = kwargs.get('w_sweep', [])
        self._sources = kwargs.get('sources', [])
        self._include_paths = kwargs.get('include_paths', [])
        self._echo = kwargs.get('echo', False)
        self._results = {}

    def run(self):
        for w in self._w_sweep:
            self._results[w] = {}
            for f in self._frequency_sweep:
                ir = InstanceRunner(
                    project=self._project,
                    frequency=f,
                    w=w,
                    sources=self._sources,
                    include_paths=self._include_paths,
                    top_level_template=self._top_level_template,
                    echo=self._echo,
                )
                ir.run()
                self._results[w][f] = ir.results()

    def results(self) -> list:
        return self._results

def run_synthesis_flow(**kwargs) -> int:
    results = {}
    for project in kwargs.get('projects', {}).keys():
        args = {
            'project': project,
            'frequency_sweep': kwargs.get('frequency_sweep', []),
            'top_level_template': kwargs.get('top_level_template'),
            'w_sweep': kwargs.get('w_sweep', []),
            'sources': kwargs['projects'][project],
            'include_paths': kwargs.get('include_paths', []),
            'echo': kwargs.get('echo', False),
        }

        r = ProjectRunner(**args)
        r.run()
        results[project] = r.results()

    return results
