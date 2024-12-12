"""Advent of Code 2024, Tag 12
Lösung von Matthias Baake
"""

from pathlib import Path

DIRECTORY = Path("2024")
PATH1 = DIRECTORY / "data_12-example1.txt"
PATH2 = DIRECTORY / "data_12-example2.txt"
PATH3 = DIRECTORY / "data_12-example3.txt"
PATH4 = DIRECTORY / "data_12-example4.txt"
PATH5 = DIRECTORY / "data_12-example5.txt"
PATH_MB = DIRECTORY / "data_12-matthias_baake.txt"

PATH = PATH_MB


def read_file(
    path: Path,
) -> tuple[dict[tuple[int, int], str], int, int]:
    """Datei einlesen

    Args:
        path: Dateiname als Path

    Returns:
        [0] Eingelesene Karte als Liste von str
        [1] Breite der Karte
        [2] Höhe der Karte
    """
    map_d: dict[tuple[int, int], str] = {}
    with open(path, encoding="utf-8") as file:
        height = 0
        for row, line in enumerate(file):
            height += 1
            width = 0
            for col, char in enumerate(line.rstrip()):
                width += 1
                map_d[(row, col)] = char

    return map_d, width, height


def get_results(
    map_d: dict[tuple[int, int], str], width: int, height: int
) -> tuple[int, int]:
    """Lösung für beide Teilaufgaben ermitteln

    Args:
        map_d: Die mit read_file eingelesene Karte
        width: Breite der Karte
        height: Höhe der Karte

    Returns:
        [0]: Lösung für Teilaufgabe 1
        [1]: Lösung für Teilaufgabe 2
    """
    # Mögliche Richtungen
    dydx_d = {"N": (-1, 0), "S": (1, 0), "W": (0, -1), "E": (0, 1)}

    # Zur Speicherung verarbeiteter Positionen
    visited_s: set[tuple[int, int]] = set()

    total_price_1 = 0
    total_price_2 = 0

    # Von allen Positionen ausgehend die dort befindlichen Flächen durchlaufen.
    for row in range(height):
        for col in range(width):
            pos = (row, col)

            # Pflanzenart an der aktuellen Position
            plant = map_d[pos]

            # Fläche aufsummieren
            area = 0

            # Ränder der Fläche speichern (Position und  die Richtung, in der
            # sich eine andere Pflanzenart oder der Rand befindet)
            perimeter_s: set[tuple[tuple[int, int], str]] = set()

            # Noch zu überprüfende Positionen, zum Start nur die aktuelle Pos.
            to_check_l = [(row, col)]

            # Solange noch Positionen existieren, die zu dem an der Ursprungspos.
            # (row, col) befindlichen Feld gehören:
            while to_check_l:
                # Eine beliebige Position aus der Menge der möglichen Pos.
                # entnehmen
                curr_pos = to_check_l.pop()

                # Wenn schon verarbeitet, dann diese nicht weiter betrachten und
                # mit der nächsten fortsetzen (wenn es noch mind. eine gibt)
                if curr_pos in visited_s:
                    continue

                # Fläche um 1 erhöhen, curr_pos als verarbeitet markieren
                area += 1
                visited_s.add(curr_pos)

                # Ausgehend von der aktuellen Position curr_pos alle möglichen
                # Richtungen ausprobieren
                for direction, (dy, dx) in dydx_d.items():
                    next_row = curr_pos[0] + dy
                    next_col = curr_pos[1] + dx
                    next_pos = (next_row, next_col)
                    if (
                        0 <= next_row < height
                        and 0 <= next_col < width
                        and map_d[next_pos] == plant
                    ):
                        # Ermittelte Position liegt noch auf der Karte und
                        # enthält die gleiche Pflanzenart. Daher zur Überprüfung
                        # in to_check_l eintragen.
                        to_check_l.append(next_pos)
                    else:
                        # Position ist außerhalb der Karte oder dort befindet
                        # sich eine andere Pflanzenart --> Position und Richtung
                        # in das Verzeichnis der Perimeter eintragen.
                        perimeter_s.add((curr_pos, direction))

            # Teilaufgabe 1: Fläche * Länge des Randes der Fläche aufaddieren
            total_price_1 += area * len(perimeter_s)

            num_sides = 0
            # Aneinander anstoßende Perimeter zusammenfassen
            while perimeter_s:
                num_sides += 1
                start_pos, direction = perimeter_s.pop()
                dy, dx = dydx_d[direction]

                pos = start_pos
                while True:
                    next_pos = (pos[0] + dx, pos[1] + dy)
                    if (next_pos, direction) in perimeter_s:
                        perimeter_s.remove((next_pos, direction))
                        pos = next_pos
                    else:
                        break

                pos = start_pos
                while True:
                    next_pos = (pos[0] - dx, pos[1] - dy)
                    if (next_pos, direction) in perimeter_s:
                        perimeter_s.remove((next_pos, direction))
                        pos = next_pos
                    else:
                        break

            # Teilaufgabe 2: Fläche * Anzahl der Seiten der Fläche aufaddieren
            total_price_2 += area * num_sides

    return total_price_1, total_price_2


def main() -> None:
    """Datei einlesen und beide Teilaufgaben lösen"""
    map_d, width, height = read_file(PATH)

    result1, result2 = get_results(map_d, width, height)
    print(f"Ergebnis Teilaufgabe 1 für {PATH}: {result1}")
    print(f"Ergebnis Teilaufgabe 2 für {PATH}: {result2}")
    # PATH1: 140, 80
    # PATH2: 772, 436
    # PATH4: (692), 236
    # PATH5: (1184), 368
    # PATH3: 1930, 1206
    # PATH_MB: 1449902, 908042


if __name__ == "__main__":
    main()
