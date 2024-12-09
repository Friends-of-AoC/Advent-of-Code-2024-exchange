"""Advent of Code 2024, Tag 9
Lösung von Matthias Baake
"""

import datetime
from pathlib import Path
from typing import Sequence

DIRECTORY = Path("2024")
PATH1 = DIRECTORY / "data_09-example.txt"
PATH2 = DIRECTORY / "data_09-matthias_baake.txt"

PATH = PATH2


def read_file(
    path: Path,
) -> tuple[list[int], list[tuple[int, int]], list[tuple[int, int]]]:
    """Datei einlesen

    Args:
        path: Dateiname als Path

    Returns:
        [0]: Eine Liste mit einem Wert pro disk-Position (-1: frei, >= 0:
             belegt mit file-Id)
        [1]: Liste von files mit Position und Länge; Index in der Liste ist
             die file-Id
        [2]: Liste von freien Bereichen mit Position und Länge
    """
    with open(path, encoding="utf-8") as file:
        len_l = map(int, next(file).rstrip())

    disk_l = []
    file_id = 0
    is_file = True
    file_l: list[tuple[int, int]] = []  # Startposition, Länge; der Index ist die Id
    free_l: list[tuple[int, int]] = []  # Startposition, Länge

    pos = 0
    for length in len_l:
        if is_file:
            disk_l.extend([file_id] * length)
            if length > 0:
                file_l.append((pos, length))
            file_id += 1
        else:
            disk_l.extend([-1] * length)
            if length > 0:
                free_l.append((pos, length))

        pos += length
        is_file = not is_file

    # Letzten Eintrag aus free_l entfernen, wenn er am Ende der disk ist,
    # da dorthin kein file verschoben werden darf
    if is_file:
        del free_l[-1]

    return disk_l, file_l, free_l


def show_disk(disk_l: Sequence[int]) -> None:
    """Gibt die Belegung der disk aus

    Args:
        disk_l: Wie der Ergebnis-Wert [0] von read_file()
    """
    print("".join(map(lambda num: str(num) if num >= 0 else ".", disk_l)))


# show_disk(disk_l)


def defrag1(disk_l: list[int]) -> list[int]:
    """Defragmentierung für Teilaufgabe 1

    Aufpassen: die übergebene Liste wird verändert!

    Args:
        disk_l: Wie der Ergebnis-Wert [0] von read_file()

    Returns:
        Wie der Ergebnis-Wert [0] von read_file()
    """
    for pos in range(len(disk_l) - 1, -1, -1):
        if (val := disk_l[pos]) >= 0:
            first_space = disk_l.index(-1)
            if first_space > pos:
                break
            disk_l[first_space] = val
            disk_l[pos] = -1

            # show_disk(disk_l)

    return disk_l


def defrag2(file_l: list[tuple[int, int]], free_l: list[tuple[int, int]]) -> list[int]:
    """Defragmentierung für Teilaufgabe 2

    Aufpassen: beide Listen werden verändert!

    Args:
        file_l: Wie der Ergebnis-Wert [1] von read_file()
        free_l: Wie der Ergebnis-Wert [2] von read_file()

    Returns:
        Wie der Ergebnis-Wert [0] von read_file()
    """
    # print(f"Anzahl files: {len(file_l)}")
    for file_id, (file_pos, file_length) in list(enumerate(file_l))[::-1]:
        # if file_id % 100 == 0:
        #     print(f"Bearbeite file {file_id}")

        # Erste Position in free_l suchen, die eine Länge von mindestens
        # length hat
        for free_index, (free_pos, free_length) in enumerate(free_l):
            if free_pos > file_pos:
                # Freie Position liegt hinter der file-Position; diese
                # und ggf. weitere freie Positionen nicht berücksichtigen
                break
            if free_length >= file_length:
                # Freien Bereich mit ausreichender Länge gefunden

                # file an neue Position setzen
                file_l[file_id] = (free_pos, file_length)

                # Freien Bereich kürzen, ggf. entfernen
                if free_length == file_length:
                    # file passte genau in den freien Bereich, dieser
                    # ist daher aufgebraucht
                    del free_l[free_index]
                else:
                    # Es sind noch free_length - file_length Positionen
                    # frei
                    free_l[free_index] = (
                        free_pos + file_length,
                        free_length - file_length,
                    )

                # An der bisherigen Position von file einen freien Bereich eintragen
                free_l.append((file_pos, file_length))

                # Liste der freien Bereiche nach der jeweils ersten Position sortieren
                free_l.sort()

                # Es kann sein, dass direkt neben dem neuen free_l-Eintrag ein
                # oder zwei andere free_l-Einträge vorhanden sind. Diese müssen
                # zu einem verschmolzen werden
                ind = 0
                while ind < len(free_l) - 1:
                    pos1, len1 = free_l[ind]
                    pos2, len2 = free_l[ind + 1]

                    if pos1 + len1 == pos2:
                        # Die nächste (ind+1) freie Position schließt direkt
                        # an die freie Position mit Index ind an --> zusammenfassen
                        free_l[ind] = (pos1, len1 + len2)
                        del free_l[ind + 1]

                        # ind nicht erhöhen (d.h. an diesem Index nochmal prüfen),
                        # da die nächste freie Position evtl. direkt benachbart ist
                    else:
                        # Weiter bei der nächsten freien Position
                        ind += 1

                # Ein file wurde verschoben, weiter mit dem nächsten file
                # (nach der Vorgabe darf jedes file nur einmal verschoben werden)
                break

    # Als Ergebnis dieser Funktion wird eine Ausgabe in der Form von Ergebnis [0]
    # von read_file benötigt; dazu aus file_l und free_l eine entsprechende
    # Liste erzeugen

    # Zunächst file_l und free_l zu einer Liste zusammenfügen, dabei die file_id
    # (bei file_l) bzw. -1 (bei free_l) als dritten Wert eintragen
    combined_l = [
        (pos, length, file_id) for file_id, (pos, length) in enumerate(file_l)
    ] + [(pos, length, -1) for (pos, length) in free_l]

    # Die so erzeugte Liste nach der Anfangsposition der Bereiche sortieren
    combined_l.sort()

    # Ergebnisliste erzeugen
    disk_l = []
    for _, length, file_id in combined_l:
        disk_l.extend([file_id] * length)

    return disk_l


def calc_checksum(disk_l: Sequence[int]) -> int:
    """Prüfsumme berechnen

    Args:
        disk_l: Wie der Ergebnis-Wert [0] von read_file()

    Returns:
        Die errechnete "Prüfsumme"
    """
    return sum(
        map(
            lambda x: x[0] * x[1] if x[1] >= 0 else 0,
            enumerate(disk_l),
        )
    )


def main() -> None:
    """Datei einlesen und beide Teilaufgaben lösen"""
    start_dt = datetime.datetime.now()
    disk_l, file_l, free_l = read_file(PATH)

    disk1_l = defrag1(disk_l[:])
    checksum = calc_checksum(disk1_l)
    print(f"Ergebnis Teilaufgabe 1 für {PATH}: {checksum}")
    end1_dt = datetime.datetime.now()
    # fname1: 1928
    # fname2: 6310675819476

    disk2_l = defrag2(file_l[:], free_l[:])
    # show_disk(disk2_l)
    checksum = calc_checksum(disk2_l)
    print(f"Ergebnis Teilaufgabe 2 für {PATH}: {checksum}")
    end2_dt = datetime.datetime.now()
    # fname1: 2858
    # fname2: 6335972980679

    print(
        "Zeit für das Einlesen der Datei und Teilaufgabe 1: "
        f"{(end1_dt-start_dt).total_seconds()} Sekunden"
    )
    # PATH1: 0.0 Sekunden
    # PATH2: 6.7 Sekunden
    print(f"Zeit für Teilaufgabe 2: {(end2_dt-end1_dt).total_seconds()} Sekunden")
    # PATH1: 0.0 Sekunden
    # PATH2: 4.7 Sekunden
    print(f"Gesamtzeit: {(end2_dt-start_dt).total_seconds()} Sekunden")
    # PATH1: 0.0 Sekunden
    # PATH2: 11.4 Sekunden


if __name__ == "__main__":
    main()
