from os import getenv
from pathlib import Path
from unittest import TestCase, skip


@skip
class TestLogGrammar(TestCase):
    def test_log_grammar(self):
        from ic4.assembler.parser import GRAMMAR
        log_file_path = Path(getenv("LOG_DIR")) / "grammar.txt"
        log_file_path.parent.mkdir(exist_ok=True, parents=True)
        with open(log_file_path, "w") as logfile:
            logfile.write(GRAMMAR)
