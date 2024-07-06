use std::collections::HashSet;
use std::path::Path;
use std::{fs::File, io::Write, path::PathBuf};

use askama::Template;
use clap::Parser;
use clap::ValueEnum;
use serde::{Deserialize, Serialize};

type Result<T> = std::result::Result<T, ()>;

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "snake_case")]
enum ValueType {
    Boolean,
    Int64,
    Float64,
    String,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(deny_unknown_fields)]
struct Flag {
    name: String,
    r#type: ValueType,
    help: String,

    #[serde(default)]
    multiple: bool,

    #[serde(default)]
    short: char,

    #[serde(default)]
    default: Option<String>,
}

impl Flag {
    fn type_name(&self) -> String {
        if self.multiple {
            match self.r#type {
                ValueType::Boolean => "boolean_array",
                ValueType::Int64 => "int64_array",
                ValueType::Float64 => "float64_array",
                ValueType::String => "string_array",
            }
        } else {
            match self.r#type {
                ValueType::Boolean => "boolean",
                ValueType::Int64 => "int64",
                ValueType::Float64 => "float64",
                ValueType::String => "string",
            }
        }
        .to_owned()
    }

    fn c_type(&self) -> String {
        if self.multiple {
            match self.r#type {
                ValueType::Boolean => "struct cfgopt_boolean_array",
                ValueType::Int64 => "struct cfgopt_int64_array",
                ValueType::Float64 => "struct cfgopt_float64_array",
                ValueType::String => "struct cfgopt_string_array",
            }
        } else {
            match self.r#type {
                ValueType::Boolean => "bool",
                ValueType::Int64 => "int64_t",
                ValueType::Float64 => "double",
                ValueType::String => "char const *",
            }
        }
        .to_owned()
    }

    fn flag_help(&self) -> String {
        let name = if self.short == '\0' {
            format!("--{}", self.name)
        } else {
            format!("-{}, --{}", self.short, self.name)
        };

        let flag_type = self.type_name();

        let flag_default = if let Some(default) = &self.default {
            format!("default: {},", default)
        } else {
            "".to_string()
        };

        let flag_help = self.help.clone();

        format!("{name} ({flag_type}) {flag_default} {flag_help}")
    }

    fn c_default(&self) -> String {
        if self.multiple {
            format!("({}) cfgopt_array_init()", self.c_type())
        } else {
            match self.r#type {
                ValueType::Boolean => "false",
                ValueType::Int64 => "0",
                ValueType::Float64 => "0.0",
                ValueType::String => "NULL",
            }
            .to_owned()
        }
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(deny_unknown_fields)]
struct Positional {
    name: String,
    r#type: ValueType,
    help: String,

    #[serde(default)]
    multiple: bool,
}

#[derive(Template, Serialize, Deserialize, Debug)]
#[template(path = "c.txt")]
#[serde(rename_all = "kebab-case", deny_unknown_fields)]
struct App {
    name: String,
    version: Option<String>,
    about: Option<String>,

    #[serde(default)]
    no_auto_help: bool,

    #[serde(default)]
    flags: Vec<Flag>,
    #[serde(default)]
    positionals: Vec<Positional>,
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

    output: Option<PathBuf>,
}

fn check_app(app: &App) -> Result<()> {
    let mut names = HashSet::new();

    for flag in &app.flags {
        if !names.insert(flag.name.clone()) {
            eprintln!("cfgopt: Duplicated flag name: {}", flag.name);
            return Err(());
        }

        if flag.short != '\0' && !names.insert(flag.short.to_string()) {
            eprintln!("cfgopt: Duplicated flag short name: {}", flag.name);
            return Err(());
        }
    }

    for pos in &app.positionals {
        if !names.insert(pos.name.to_string()) {
            eprintln!("cfgopt: Duplicated positional name: {}", pos.name);
            return Err(());
        }
    }

    Ok(())
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

    let mut app: App = toml::from_str(&cfg)
        .map_err(|err| eprintln!("cfgopt: Failed to parse config file {config:?}: {err}"))?;

    if !app.no_auto_help {
        app.flags.push(Flag {
            name: "help".to_owned(),
            r#type: ValueType::Boolean,
            help: "print this help and exit".to_string(),
            multiple: false,
            short: 'h',
            default: None,
        })
    }

    check_app(&app)?;

    if let Some(output) = &cli.output {
        let mut fout =
            File::create(output).map_err(|err| eprintln!("cfgopt: Failed write output: {err}"))?;
        write!(&mut fout, "{}", app.render().unwrap())
            .map_err(|err| eprintln!("cfgopt: Failed write output: {err}"))?;
    } else {
        print!("{}", app.render().unwrap());
    };

    Ok(())
}
