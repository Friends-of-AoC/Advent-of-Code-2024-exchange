// I quickly saw that this day's tasks can be seen as another instance of a graph and solved with
// shortest-path / dijkstra.
//
// I tried preparing  all shortest-paths, but realized afterward due the need of recursion,
// this cant be pre-pared and for each button press alone a shortest-path for every layer
// need to be found. This includes doing dijkstra on each and every level for each button press.
// (Remark: A rust-solution for this approach is in [1])
//
// I kinda was too much stuck before realizing this, thus tried to find a whole other solution
// looking into Reddit. Some solutions had kind of shortest-path-rules, e.g. avoid zig-zag,
// but I was unable to adopt my present code, without the recursive dijkstra, to work fully.
//
// Therefore, I go here with another solution, also found on Reddit,  where the heavy lifting
// is done as manual calculated shortest-path map by the author of that solution.
//
// See: https://www.reddit.com/r/adventofcode/comments/1hj2odw/comment/m36j01x/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
// See: https://topaz.github.io/paste/#XQAAAQD6EQAAAAAAAAA4GEiZzRd1JAgz+whYRQxSFI7XvmlfhtGDinguAj8sFyD4ftJ8OW5ekoqnVIaEYm1TRzRozXdrSWph6uPxJTZjFA79rnO0DuC0UuIh7ERk/Duk3psptqVA73J8Z4I2hW2SL6gJB3Q1/1XR38DhDEO8md9rWyVWBo4CJDydIWxuyMJcWVQ1ufgwX0ZmJoE5ZQfJjzbh+DZJ+rn4Sbosya9WNQ6/9qJVmsYOKTaGQSnTXeXuKSxXcsEM8aOYrwwokHmz1qm/XBD0xY3AAVJTkzrYdTl4OQox1mjh84ro0qEU4/rUv+M6fqUGeLv9d6MjvSvdq9zb3kVlUg0EIXhQNEfOjoUDdnZjwo6W1fT4LIOQRKQDTXkLNCovRuWLphjKGCHFmcJyrSRLcBrpa3UPBz2cIWDABX80asng9GqkqlBPZ00Xwo7HcF2g/+CbHp5krjy5MghjW+IFMXZ6KzO3bqOGmmXmXmAJbcPpaASmOyJuZAIqZPlGWT8cfySbmkncDrAdGAl5s9SaZp+ouygkV74I+YpoBLf0T+bbJ4jHatgdyEeX+hb35+eN08OB0E0S06WUzSAHJfIJV6QqkQAhlu3aJTiam9LsuRQa8l65jvSVum1zympU0thS5R73tSq43ExG3n7ZoPEt54+GoAqWEb8DYTiiSaLSL0E+aDYNsxeBhJNU8aw3+TPNHS5AOzEHuKfY1fAfcvsx8tcpeEB1bSGtPhmjbDOJtnzhdS76jrV68K4hjgzpqJ/YfVeoIhVBPJpnc9AuNMyMECjKIN9gOhKqzooSr6teDDrCi6EFCvwAWoA9BQtKq+m4nH0QPUznsSCp4WVVzzeuuUQU9VKbtJClZlWGdFOp+iaoaZg3jec+rsSrPbW/Egz9Wz6G+BT+ehdYjxUjkSj27qxJK0O+0prbEkq1E4rjNgZIq0l1n76ERO7TPmYhDOzOP/URcnhJJ27nB6eeeABAlr7ovC+1nFP51FNvugYf8NvGQ1T5Tk/X1yGvomNHwHvvJ6PWDQn4FgDPQj1f6AEqnD1aStgkAszG7CSoF1shmf6ODrEcuELJ+UcVUrD2QFMiIqpi5xaez/noF5INiALPoYedsYG9cQ1ogzqBK80652Q2JK7YxdAX9N2ANeYF9tdZ6kY4Gh4t9sIa16T1ujIPde/SFaEFy/uxVkjwMsxUMdLCyIKJJ9YtPI9Ahfmt+3HziHQHM3IQ8sqTlPnyFPp2JLazKBkPlOuaZ1Q+xFRmTf5urms6GyI4GZ6nsjRCoGqJ391vaD8LhgSUlbTVPLgPolti1H4WVKv/DMee2bSRnpmlbff9/BzPP/qbip1F1iw2Za9WZ7ycf4zMn2/zK0VW8OtNAfRTStX3wqWNStrdPSPppYtRSnsHTkGYND3q5lLI0JyEJNu11IOiM2XQMVzF6QgTOuiCx8qKdFG+pUtWL2onkj2iNOvURnsU0jTqWH2rCVdG6T9JeXEjZno7bABLfBmyMNBB+v9ajn+Nv0pW7Ag9cHPgeogkZVvx6zaJTpPQQ/cD2juyVFpEsdBmXEqaIVrKOCIUYnZUwEmEF3zeDqdJZqlYwrQcQzD4xX4SpDxWRXDTR24Rg7zj/vuA/eO1eb4IA/qLxhltMAb7VLqZOU0imaKmDZHOjTP14WoRXD/uy6Y7p9BThBCmg+fX3QXFu5HsCgN82aaAbKJ6laKX2CbA6//SMmnE
// [1] See: https://nickymeuleman.netlify.app/blog/aoc2024-day21/
use std::collections::HashMap;
use std::fs::File;
use std::io::BufRead;
use std::path::Path;
use std::{env, io};

fn main() -> Result<(), String> {
    // Arg parsing
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        return Err(String::from("Usage: cargo run --bin day18 INPUT"));
    }

    // Prepare file for reading
    let input_path = Path::new(&args[1]);
    let file =
        File::open(input_path).map_err(|_| String::from("Failed to open the INPUT file."))?;
    let reader = io::BufReader::new(file);

    // Load and parse input from file
    let codes = reader.lines().collect::<Result<Vec<_>, _>>().unwrap();

    // Helpers
    let paths = shortest_paths();
    let mut cache: HashMap<(String, u64), u64> = HashMap::new();

    // Task 1:
    // What is the sum of the complexities of the five codes on your list [via 3 robots]?
    let mut total_complexity: u64 = 0;
    for code in codes.to_owned() {
        let numeric_part = code
            .chars()
            .filter(|c| c.is_digit(10))
            .collect::<String>()
            .parse::<u64>()
            .unwrap();
        let seq_len = sequence_length(code, 3, &paths, &mut cache);
        let complexity = seq_len * numeric_part;
        total_complexity += complexity;
    }
    println!("Task 1: {}", total_complexity);

    // Task 1:
    // What is the sum of the complexities of the five codes on your list [via 26 robots]?
    let mut total_complexity: u64 = 0;
    for code in codes {
        let numeric_part = code
            .chars()
            .filter(|c| c.is_digit(10))
            .collect::<String>()
            .parse::<u64>()
            .unwrap();
        let seq_len = sequence_length(code, 26, &paths, &mut cache);
        let complexity = seq_len * numeric_part;
        total_complexity += complexity;
    }
    println!("Task 2: {}", total_complexity);

    Ok(())
}

// Remark: See the comment at the top of this file, the logic is taken from:
// See: https://www.reddit.com/r/adventofcode/comments/1hj2odw/comment/m36j01x/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
fn sequence_length(
    sequence: String,
    depth: u64,
    shortest_paths: &HashMap<(char, char), String>,
    cache: &mut HashMap<(String, u64), u64>,
) -> u64 {
    let key = (sequence.to_owned(), depth);
    if cache.contains_key(&key) {
        return cache[&key];
    }

    let mut length: u64 = 0;
    if depth == 0 {
        length = sequence.len() as u64;
    } else {
        let mut current = 'A';
        for next in sequence.chars() {
            // same button, just hit it again
            if current == next {
                length += 1;
            // take the prepared shortest paths
            } else {
                let sub_sequence = shortest_paths[&(current, next)].to_owned();
                length += sequence_length(sub_sequence, depth - 1, shortest_paths, cache);
            }
            // move along the sequence
            current = next;
        }
    }

    cache.insert(key, length);

    length
}


// Remark: See the comment at the top of this file, this hand-crafted/calculated shortest-path
// map is taken from:
// See: https://www.reddit.com/r/adventofcode/comments/1hj2odw/comment/m36j01x/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
fn shortest_paths<'a>() -> HashMap<(char, char), String> {
    HashMap::from([
        (('A', '0'), "<A".to_owned().to_owned()),
        (('0', 'A'), ">A".to_owned()),
        (('A', '1'), "^<<A".to_owned()),
        (('1', 'A'), ">>vA".to_owned()),
        (('A', '2'), "<^A".to_owned()),
        (('2', 'A'), "v>A".to_owned()),
        (('A', '3'), "^A".to_owned()),
        (('3', 'A'), "vA".to_owned()),
        (('A', '4'), "^^<<A".to_owned()),
        (('4', 'A'), ">>vvA".to_owned()),
        (('A', '5'), "<^^A".to_owned()),
        (('5', 'A'), "vv>A".to_owned()),
        (('A', '6'), "^^A".to_owned()),
        (('6', 'A'), "vvA".to_owned()),
        (('A', '7'), "^^^<<A".to_owned()),
        (('7', 'A'), ">>vvvA".to_owned()),
        (('A', '8'), "<^^^A".to_owned()),
        (('8', 'A'), "vvv>A".to_owned()),
        (('A', '9'), "^^^A".to_owned()),
        (('9', 'A'), "vvvA".to_owned()),
        (('0', '1'), "^<A".to_owned()),
        (('1', '0'), ">vA".to_owned()),
        (('0', '2'), "^A".to_owned()),
        (('2', '0'), "vA".to_owned()),
        (('0', '3'), "^>A".to_owned()),
        (('3', '0'), "<vA".to_owned()),
        (('0', '4'), "^<^A".to_owned()),
        (('4', '0'), ">vvA".to_owned()),
        (('0', '5'), "^^A".to_owned()),
        (('5', '0'), "vvA".to_owned()),
        (('0', '6'), "^^>A".to_owned()),
        (('6', '0'), "<vvA".to_owned()),
        (('0', '7'), "^^^<A".to_owned()),
        (('7', '0'), ">vvvA".to_owned()),
        (('0', '8'), "^^^A".to_owned()),
        (('8', '0'), "vvvA".to_owned()),
        (('0', '9'), "^^^>A".to_owned()),
        (('9', '0'), "<vvvA".to_owned()),
        (('1', '2'), ">A".to_owned()),
        (('2', '1'), "<A".to_owned()),
        (('1', '3'), ">>A".to_owned()),
        (('3', '1'), "<<A".to_owned()),
        (('1', '4'), "^A".to_owned()),
        (('4', '1'), "vA".to_owned()),
        (('1', '5'), "^>A".to_owned()),
        (('5', '1'), "<vA".to_owned()),
        (('1', '6'), "^>>A".to_owned()),
        (('6', '1'), "<<vA".to_owned()),
        (('1', '7'), "^^A".to_owned()),
        (('7', '1'), "vvA".to_owned()),
        (('1', '8'), "^^>A".to_owned()),
        (('8', '1'), "<vvA".to_owned()),
        (('1', '9'), "^^>>A".to_owned()),
        (('9', '1'), "<<vvA".to_owned()),
        (('2', '3'), ">A".to_owned()),
        (('3', '2'), "<A".to_owned()),
        (('2', '4'), "<^A".to_owned()),
        (('4', '2'), "v>A".to_owned()),
        (('2', '5'), "^A".to_owned()),
        (('5', '2'), "vA".to_owned()),
        (('2', '6'), "^>A".to_owned()),
        (('6', '2'), "<vA".to_owned()),
        (('2', '7'), "<^^A".to_owned()),
        (('7', '2'), "vv>A".to_owned()),
        (('2', '8'), "^^A".to_owned()),
        (('8', '2'), "vvA".to_owned()),
        (('2', '9'), "^^>A".to_owned()),
        (('9', '2'), "<vvA".to_owned()),
        (('3', '4'), "<<^A".to_owned()),
        (('4', '3'), "v>>A".to_owned()),
        (('3', '5'), "<^A".to_owned()),
        (('5', '3'), "v>A".to_owned()),
        (('3', '6'), "^A".to_owned()),
        (('6', '3'), "vA".to_owned()),
        (('3', '7'), "<<^^A".to_owned()),
        (('7', '3'), "vv>>A".to_owned()),
        (('3', '8'), "<^^A".to_owned()),
        (('8', '3'), "vv>A".to_owned()),
        (('3', '9'), "^^A".to_owned()),
        (('9', '3'), "vvA".to_owned()),
        (('4', '5'), ">A".to_owned()),
        (('5', '4'), "<A".to_owned()),
        (('4', '6'), ">>A".to_owned()),
        (('6', '4'), "<<A".to_owned()),
        (('4', '7'), "^A".to_owned()),
        (('7', '4'), "vA".to_owned()),
        (('4', '8'), "^>A".to_owned()),
        (('8', '4'), "<vA".to_owned()),
        (('4', '9'), "^>>A".to_owned()),
        (('9', '4'), "<<vA".to_owned()),
        (('5', '6'), ">A".to_owned()),
        (('6', '5'), "<A".to_owned()),
        (('5', '7'), "<^A".to_owned()),
        (('7', '5'), "v>A".to_owned()),
        (('5', '8'), "^A".to_owned()),
        (('8', '5'), "vA".to_owned()),
        (('5', '9'), "^>A".to_owned()),
        (('9', '5'), "<vA".to_owned()),
        (('6', '7'), "<<^A".to_owned()),
        (('7', '6'), "v>>A".to_owned()),
        (('6', '8'), "<^A".to_owned()),
        (('8', '6'), "v>A".to_owned()),
        (('6', '9'), "^A".to_owned()),
        (('9', '6'), "vA".to_owned()),
        (('7', '8'), ">A".to_owned()),
        (('8', '7'), "<A".to_owned()),
        (('7', '9'), ">>A".to_owned()),
        (('9', '7'), "<<A".to_owned()),
        (('8', '9'), ">A".to_owned()),
        (('9', '8'), "<A".to_owned()),
        (('<', '^'), ">^A".to_owned()),
        (('^', '<'), "v<A".to_owned()),
        (('<', 'v'), ">A".to_owned()),
        (('v', '<'), "<A".to_owned()),
        (('<', '>'), ">>A".to_owned()),
        (('>', '<'), "<<A".to_owned()),
        (('<', 'A'), ">>^A".to_owned()),
        (('A', '<'), "v<<A".to_owned()),
        (('^', 'v'), "vA".to_owned()),
        (('v', '^'), "^A".to_owned()),
        (('^', '>'), "v>A".to_owned()),
        (('>', '^'), "<^A".to_owned()),
        (('^', 'A'), ">A".to_owned()),
        (('A', '^'), "<A".to_owned()),
        (('v', '>'), ">A".to_owned()),
        (('>', 'v'), "<A".to_owned()),
        (('v', 'A'), "^>A".to_owned()),
        (('A', 'v'), "<vA".to_owned()),
        (('>', 'A'), "^A".to_owned()),
        (('A', '>'), "vA".to_owned()),
    ])
}
