use askama::Template;
use std::{fs::File, io::Write};

use serde::{Deserialize, Serialize};

type Result<T> = std::result::Result<T, ()>;

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "snake_case")]
enum FlagType {
    Boolean,
    Int64,
    Float64,
    String,
}

impl FlagType {
    fn c_type(&self) -> String {
        match self {
            FlagType::Boolean => "bool",
            FlagType::Int64 => "int64_t",
            FlagType::Float64 => "double",
            FlagType::String => "char const *",
        }
        .to_owned()
    }

    fn c_default(&self) -> String {
        match self {
            FlagType::Boolean => "false",
            FlagType::Int64 => "0",
            FlagType::Float64 => "0.0",
            FlagType::String => "NULL",
        }
        .to_owned()
    }

    fn name(&self) -> String {
        match self {
            FlagType::Boolean => "boolean",
            FlagType::Int64 => "int64",
            FlagType::Float64 => "float64",
            FlagType::String => "string",
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
#[template(path = "c.txt")]
struct App {
    name: String,
    version: Option<String>,
    about: Option<String>,
    flags: Vec<Flag>,
}

fn main() -> Result<()> {
    let cfg = std::fs::read_to_string("cfgopt.toml").unwrap();
    let app: App = toml::from_str(&cfg).unwrap();

    let mut fout = File::create("cfgopt_gen.h").unwrap();
    write!(&mut fout, "{}", app.render().unwrap()).unwrap();

    Ok(())
}
