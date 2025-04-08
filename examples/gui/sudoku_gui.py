try:
    from sd_solver import solve
except ImportError as e:
    print("❌ You need to build or install the `sd_solver` Cython extension first.")
    print("   Run: `python setup.py build_ext --inplace` or `pip install .`")
    raise e

try:
    from PySide6.QtWidgets import QApplication
except ImportError as e:
    print("❌ Missing PySide6. Install it via:")
    print("   pip install PySide6")
    raise e

import sys
import numpy as np
from PySide6.QtWidgets import (
    QApplication, QWidget, QGridLayout, QPushButton,
    QVBoxLayout, QHBoxLayout, QLabel, QMessageBox
)
from PySide6.QtCore import Qt
from PySide6.QtGui import QFont
from PySide6.QtWidgets import QSizePolicy
from PySide6.QtWidgets import QScrollArea

import sd_solver

class SudokuCell(QLabel):
    def __init__(self, row, col, value=0, parent=None):
        super().__init__(parent)
        self.row = row
        self.col = col
        self.setFixedSize(50, 50)
        self.setAlignment(Qt.AlignCenter)
        self.setFont(QFont("Arial", 20))
        self.setStyleSheet("border: 1px solid black;")
        self.value = value
        self.setText("" if value == 0 else str(value))
        self.setFocusPolicy(Qt.StrongFocus)

    def setValue(self, val):
        self.value = val
        self.setText("" if val == 0 else str(val))

    def keyPressEvent(self, event):
        key = event.key()
        if Qt.Key_0 <= key <= Qt.Key_9:
            num = key - Qt.Key_0
            self.setValue(num)
        elif key in (Qt.Key_Up, Qt.Key_Down, Qt.Key_Left, Qt.Key_Right):
            self.move_focus(key)

    def move_focus(self, key):
        parent = self.parent()
        if not hasattr(parent, "board"):
            return

        r, c = self.row, self.col
        if key == Qt.Key_Up:
            r = (r - 1) % 9
        elif key == Qt.Key_Down:
            r = (r + 1) % 9
        elif key == Qt.Key_Left:
            c = (c - 1) % 9
        elif key == Qt.Key_Right:
            c = (c + 1) % 9

        next_cell = parent.board[r][c]
        next_cell.setFocus()

    def focusInEvent(self, event):
        self.setStyleSheet("border: 2px solid #3776AB;")
        super().focusInEvent(event)

    def focusOutEvent(self, event):
        self.setStyleSheet("border: 1px solid black;")
        super().focusOutEvent(event)
def make_insert_callback(n):
    def callback():
        print(f"[DEBUG] Button {n} clicked")
        widget = self.focusWidget()
        if isinstance(widget, SudokuCell):
            print(f"[DEBUG] Focused cell: ({widget.row}, {widget.col})")
            widget.setValue(n)
        else:
            print("[WARN] No SudokuCell focused!")
    return callback
class SudokuApp(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Sudoku · PySide6")
        self.board = [[SudokuCell(r, c) for c in range(9)] for r in range(9)]
        self.init_ui()
        screen_rect = QApplication.primaryScreen().availableGeometry()
        self.setMinimumSize(400, 400)
        self.resize(min(600, screen_rect.width()), min(700, screen_rect.height()))

    def init_ui(self):
        main_layout = QVBoxLayout()

        # ---- Sudoku Grid ----
        grid_widget = QWidget()
        grid_layout = QGridLayout(grid_widget)
        for r in range(9):
            for c in range(9):
                cell = self.board[r][c]
                cell.setMinimumSize(25, 25)
                cell.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
                grid_layout.addWidget(cell, r, c)

        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)
        scroll_area.setWidget(grid_widget)
        scroll_area.setMinimumHeight(300)
        main_layout.addWidget(scroll_area)

        # ---- Virtual Keypad ----
        keypad_layout = QHBoxLayout()
        for i in range(10):
            btn = QPushButton(str(i))
            btn.setMinimumSize(25, 25)
            btn.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
            btn.clicked.connect(lambda _, n=i: self.insert_number(n))
            btn.setFocusPolicy(Qt.NoFocus)
            keypad_layout.addWidget(btn)
        main_layout.addLayout(keypad_layout)

        # ---- Control Buttons ----
        control_layout = QHBoxLayout()
        solve_btn = QPushButton("Solve")
        solve_btn.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        solve_btn.clicked.connect(self.solve_puzzle)

        clear_btn = QPushButton("Clear")
        clear_btn.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        clear_btn.clicked.connect(self.clear_board)

        control_layout.addWidget(solve_btn)
        control_layout.addWidget(clear_btn)
        main_layout.addLayout(control_layout)

        # ---- Hint Label ----
        hint_label = QLabel("Press '0' or click [0] to clear a cell.")
        hint_label.setAlignment(Qt.AlignCenter)
        hint_label.setStyleSheet("color: gray; font-size: 12px; margin-top: 6px;")
        main_layout.addWidget(hint_label)

        # ---- Apply Layout ----
        self.setLayout(main_layout)

    def insert_number(self, num):
        widget = self.focusWidget()
        if isinstance(widget, SudokuCell):
            widget.setValue(num)

    def clear_board(self):
        for row in self.board:
            for cell in row:
                cell.setValue(0)

    def get_puzzle_array(self):
        arr = np.zeros(81, dtype=np.int8)
        for r in range(9):
            for c in range(9):
                arr[r * 9 + c] = self.board[r][c].value
        return arr

    def update_board(self, array_2d):
        for r in range(9):
            for c in range(9):
                self.board[r][c].setValue(array_2d[r, c])

    def solve_puzzle(self):
        try:
            puzzle = self.get_puzzle_array()
            solved = sd_solver.solve(puzzle)
            self.update_board(solved)
        except Exception as e:
            QMessageBox.critical(self, "Error", str(e))


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = SudokuApp()
    window.show()
    sys.exit(app.exec())
