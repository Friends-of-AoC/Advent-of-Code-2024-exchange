// Part 2's logic is taken from a Reddit solution.
// I missed some knowledge in Full Adder to see another approach than brute-forcing by reading
// the task.
// And brute-forcing would be around 500^8 iterations :p
// Source: https://www.reddit.com/r/adventofcode/comments/1hl698z/comment/m3lnhrw
// Source: https://github.com/Gmark2000/advent-of-code-2024-MarkGozner/blob/main/Day24/Program.cs#L112-L151

use regex::Regex;
use std::collections::{HashMap, HashSet, VecDeque};
use std::path::Path;
use std::{env, fs};

const INPUT_REGEX: &str = r"(?m)(?<init>^(?<wire>[0-9a-z]{3}): (?<val>[01])$)|(?<gates>^(?<input1>[0-9a-z]{3}) (?<op>[A-Z]{2,3}) (?<input2>[0-9a-z]{3}) -> (?<output>[0-9a-z]{3})$)";

type Wire = String;

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
enum Operand {
    AND,
    OR,
    XOR,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
struct Gate {
    input1: String,
    input2: String,
    output: String,
    operand: Operand,
}

fn main() -> Result<(), String> {
    // Arg parsing
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        return Err(String::from("Usage: cargo run --bin day24 INPUT"));
    }

    // Load file into string
    let input_path = Path::new(&args[1]);
    let input = fs::read_to_string(input_path).unwrap();

    // The device
    let mut init_state: HashMap<Wire, bool> = HashMap::new();
    let mut gates: VecDeque<Gate> = VecDeque::new();

    // Parse device description from input
    let re = Regex::new(INPUT_REGEX).unwrap();
    for caps in re.captures_iter(&*input) {
        match caps.name("init") {
            Some(_) => {
                let wire = caps.name("wire").unwrap().as_str().to_string();
                let val = match caps.name("val").unwrap().as_str() {
                    "0" => false,
                    "1" => true,
                    _ => panic!("Unknown val: {}", caps.name("val").unwrap().as_str()),
                };
                init_state.insert(wire, val);
            }
            None => {
                let input1 = caps.name("input1").unwrap().as_str().to_string();
                let input2 = caps.name("input2").unwrap().as_str().to_string();
                let output = caps.name("output").unwrap().as_str().to_string();
                let operand = match caps.name("op").unwrap().as_str() {
                    "AND" => Operand::AND,
                    "OR" => Operand::OR,
                    "XOR" => Operand::XOR,
                    _ => panic!("Unknown op: {}", caps.name("op").unwrap().as_str()),
                };
                gates.push_back(Gate {
                    input1,
                    input2,
                    operand,
                    output,
                });
            }
        }
    }

    // Task 1:
    // Simulate the system of gates and wires.
    // What decimal number does it output on the wires starting with z?
    let state1 = simulate(&init_state, &gates);
    let task1 = decode(&state1, 'z');
    println!("Task1: {}", task1);

    // Task 2:
    // What do you get if you sort the names of the eight wires involved in a swap and then
    // join those names with commas?
    // The logic is completely taken from this shared solution from Reddit:
    // Source: https://www.reddit.com/r/adventofcode/comments/1hl698z/comment/m3lnhrw
    // Source: https://github.com/Gmark2000/advent-of-code-2024-MarkGozner/blob/main/Day24/Program.cs#L112-L151
    let mut swapped_outputs: HashSet<String> = HashSet::new();

    let mut z_wires = gates
        .iter()
        .filter(|gate| gate.output.starts_with('z'))
        .map(|gate| gate.output.to_string())
        .collect::<Vec<String>>();
    z_wires.sort();
    let last_z_wire = z_wires.pop().unwrap();

    for gate in &gates {
        let mut is_faulty = false;

        if gate.output.starts_with('z') && gate.output != last_z_wire {
            is_faulty = gate.operand != Operand::XOR;
        } else if !gate.output.starts_with('z')
            && !(gate.input1.starts_with('x') || gate.input1.starts_with('y'))
            && !(gate.input2.starts_with('x') || gate.input2.starts_with('y'))
        {
            is_faulty = gate.operand == Operand::XOR;
        } else if (gate.input1.starts_with('x') || gate.input1.starts_with('y'))
            && (gate.input2.starts_with('x') || gate.input2.starts_with('y'))
            && !(gate.input1.ends_with("00") && gate.input2.ends_with("00"))
        {
            let output = gate.output.to_owned();
            let expected_next_type = if gate.operand == Operand::XOR {
                Operand::XOR
            } else {
                Operand::OR
            };
            let feeds_into_expected_gate = gates.iter().any(|other| {
                other != gate
                    && (other.input1 == output || other.input2 == output)
                    && other.operand == expected_next_type
            });
            is_faulty = !feeds_into_expected_gate;
        }

        if is_faulty {
            swapped_outputs.insert(gate.output.to_owned());
        }
    }

    let mut task2 = Vec::from_iter(swapped_outputs);
    task2.sort();
    println!("Task2: {}", task2.join(","));

    // Brute-force solution, too slow, as we have around 500^8 possibilities.
    // let mut swapped_outputs: Vec<String> = Vec::new();
    // for gate1 in 0..gates.len() {
    //     for gate2 in 0..gates.len() {
    //         if gate1 == gate2 {
    //             continue;
    //         }
    //         for gate3 in 0..gates.len() {
    //             if gate2 == gate3 {
    //                 continue;
    //             }
    //             for gate4 in 0..gates.len() {
    //                 if gate3 == gate4 {
    //                     continue;
    //                 }
    //                 for gate5 in 0..gates.len() {
    //                     if gate4 == gate5 {
    //                         continue;
    //                     }
    //                     for gate6 in 0..gates.len() {
    //                         if gate5 == gate6 {
    //                             continue;
    //                         }
    //                         for gate7 in 0..gates.len() {
    //                             if gate6 == gate7 {
    //                                 continue;
    //                             }
    //                             for gate8 in 0..gates.len() {
    //                                 if gate7 == gate8 {
    //                                     continue;
    //                                 }
    //                                 let mut swapped_gates = gates.clone();
    //                                 swap_outputs(&mut swapped_gates, gate1, gate2);
    //                                 swap_outputs(&mut swapped_gates, gate3, gate4);
    //                                 swap_outputs(&mut swapped_gates, gate5, gate6);
    //                                 swap_outputs(&mut swapped_gates, gate7, gate8);
    //                                 let state2 = simulate(&init_state, &swapped_gates);
    //                                 if check_addition(&state2) {
    //                                     swapped_outputs.push(gates[gate1].output.to_owned());
    //                                     swapped_outputs.push(gates[gate2].output.to_owned());
    //                                     swapped_outputs.push(gates[gate3].output.to_owned());
    //                                     swapped_outputs.push(gates[gate4].output.to_owned());
    //                                     swapped_outputs.push(gates[gate5].output.to_owned());
    //                                     swapped_outputs.push(gates[gate6].output.to_owned());
    //                                     swapped_outputs.push(gates[gate7].output.to_owned());
    //                                     swapped_outputs.push(gates[gate8].output.to_owned());
    //                                     break;
    //                                 }
    //                             }
    //                         }
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }
    // swapped_outputs.sort();
    // println!("Task2: {}", swapped_outputs.join(","));

    Ok(())
}

fn simulate(init_state: &HashMap<Wire, bool>, gates: &VecDeque<Gate>) -> HashMap<Wire, bool> {
    let mut state = init_state.clone();
    let mut queue = gates.clone();
    while let Some(gate) = queue.pop_front() {
        // If one of the inputs not yet evaluated, then put it at the end of the queue.
        if state.get(&gate.input1) == None || state.get(&gate.input2) == None {
            queue.push_back(gate);
            continue;
        }
        // Inputs ready, evaluate
        let input1 = state.get(&gate.input1).unwrap();
        let input2 = state.get(&gate.input2).unwrap();
        let output_val = match gate.operand {
            Operand::AND => *input1 && *input2,
            Operand::OR => *input1 || *input2,
            Operand::XOR => *input1 ^ *input2,
        };
        state.insert(gate.output, output_val);
    }
    state
}

fn decode(state: &HashMap<Wire, bool>, prefix: char) -> u64 {
    let mut i = 0;
    let mut output = 0;
    while let Some(val) = state.get(format!("{}{:02}", prefix, i).as_str()) {
        // if val is false, then we don't need to add something to the result
        if *val {
            output += 2u64.pow(i)
        }
        // check the next val
        i += 1;
    }
    output
}

// For the too slow brute force solution of part 2.
// fn check_addition(state: &HashMap<Wire, bool>) -> bool {
//     let x = decode(state, 'x');
//     let y = decode(state, 'y');
//     let z = decode(state, 'z');
//
//     x + y == z
// }
//
// fn swap_outputs(gates: &mut VecDeque<Gate>, gate1: usize, gate2: usize) {
//     let x = gates[gate1].output.to_owned();
//     let y = gates[gate2].output.to_owned();
//     gates[gate1].output = y;
//     gates[gate2].output = x;
// }
