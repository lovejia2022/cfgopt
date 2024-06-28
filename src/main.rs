use askama::Template;
use std::{fs::File, io::Write};

use serde::{Deserialize, Serialize};

type Result<T> = std::result::Result<T, ()>;

#[derive(Serialize, Deserialize, Debug)]
enum FlagType {
    Bool,
    I64,
    F64,
    Str,
}

impl FlagType {
    fn c_type(&self) -> String {
        match self {
            FlagType::Bool => "bool",
            FlagType::I64 => "int64_t",
            FlagType::F64 => "double",
            FlagType::Str => "char const *",
        }
        .to_owned()
    }

    fn c_default(&self) -> String {
        match self {
            FlagType::Bool => "false",
            FlagType::I64 => "0",
            FlagType::F64 => "0.0",
            FlagType::Str => "NULL",
        }
        .to_owned()
    }

    fn name(&self) -> String {
        match self {
            FlagType::Bool => "bool",
            FlagType::I64 => "int64",
            FlagType::F64 => "float64",
            FlagType::Str => "string",
        }
        .to_owned()
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(deny_unknown_fields)]
struct Flag {
    name: String,
    r#type: FlagType,
    help: String,

    #[serde(default)]
    value_name: Option<String>,

    #[serde(default)]
    alias: Vec<String>,

    #[serde(default)]
    default: Option<String>,

    #[serde(default)]
    env: Option<String>,
}

#[derive(Template, Serialize, Deserialize, Debug)]
#[template(path = "app.txt")]
struct App {
    name: String,
    version: Option<String>,
    about: Option<String>,
    flags: Vec<Flag>,
}

impl App {}

fn main() -> Result<()> {
    let cfg = std::fs::read_to_string("cfgopt.toml").unwrap();
    let app: App = toml::from_str(&cfg).unwrap();

    let mut fout = File::create("cfgopt_gen.h").unwrap();
    write!(&mut fout, "{}", app.render().unwrap()).unwrap();

    Ok(())
}
