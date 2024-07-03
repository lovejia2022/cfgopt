use std::path::Path;
use std::{fs::File, io::Write, path::PathBuf};

use askama::Template;
use clap::Parser;
use clap::ValueEnum;
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
    short: char,

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

#[derive(Clone, Copy, ValueEnum)]
enum Language {
    C,
}

#[derive(Parser)]
struct CommandLine {
    #[clap(short, long)]
    config: Option<PathBuf>,

    #[clap(short, long)]
    language: Language,

    output: PathBuf,
}

fn main() -> Result<()> {
    let cli = CommandLine::parse();

    let config = cli
        .config
        .as_ref()
        .map(PathBuf::as_path)
        .unwrap_or_else(|| Path::new("cfgopt.toml"));

    let cfg = std::fs::read_to_string(config)
        .map_err(|err| eprintln!("cfgopt: Failed to open config file {config:?}: {err}"))?;

    let app: App = toml::from_str(&cfg)
        .map_err(|err| eprintln!("cfgopt: Failed to parse config file {config:?}: {err}"))?;

    let mut fout =
        File::create(&cli.output).map_err(|err| eprintln!("cfgopt: Failed write output: {err}"))?;

    write!(&mut fout, "{}", app.render().unwrap())
        .map_err(|err| eprintln!("cfgopt: Failed write output: {err}"))?;

    Ok(())
}
