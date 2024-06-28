use std::io::Write;

use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
struct App {
    name: String,
    version: Option<String>,
    about: Option<String>,
    flags: Vec<Flag>,
}

impl App {
    fn c_struct<W: Write>(&self, w: &mut W) -> std::io::Result<()> {
        writeln!(w, "struct cfg_opt_{} {{", self.name)?;
        for flag in &self.flags {
            writeln!(w, "\t{} {};", flag.r#type.c_type(), flag.name)?;
        }
        writeln!(w, "}};")?;
        Ok(())
    }

    fn c_init<W: Write>(&self, w: &mut W) -> std::io::Result<()> {
        writeln!(
            w,
            "static inline void cfg_opt_{name}_init(struct cfg_opt_{name} *cfg_opt_{name}_v)",
            name = self.name
        )?;
        writeln!(w, "{{")?;
        for flag in &self.flags {
            write!(w, "\tcfg_opt_{name}_v->{name} = ", name = flag.name,)?;

            if let Some(defval) = &flag.default {
                write!(w, "{}", defval)?
            } else {
                match flag.r#type {
                    FlagType::Bool => write!(w, "false"),
                    FlagType::I64 => write!(w, "0"),
                    FlagType::F64 => write!(w, "0.0"),
                    FlagType::Str => write!(w, "NULL"),
                    FlagType::Enum => write!(w, "TODO"),
                }?;
            }

            writeln!(w, ";")?;
        }
        writeln!(w, "}}")?;
        Ok(())
    }

    fn c_parse<W: Write>(&self, w: &mut W) -> std::io::Result<()> {
        writeln!(
            w,
            concat!(
                "static inline void cfg_opt_{name}_parse(\n",
                "\tstruct cfg_opt_{name} *cfg_opt_{name}_v,\n",
                "\tint argc,\n",
                "\tchar **argv)"
            ),
            name = self.name
        )?;
        writeln!(w, "{{")?;
        writeln!(
            w,
            "\tcfg_opt_{name}_init(cfg_opt_{name}_v);",
            name = self.name
        )?;
        writeln!(w, "}}")?;
        Ok(())
    }
}

#[derive(Serialize, Deserialize, Debug)]
enum FlagType {
    Bool,
    I64,
    F64,
    Str,
    Enum,
}

impl FlagType {
    fn c_type(&self) -> String {
        match self {
            FlagType::Bool => "bool",
            FlagType::I64 => "int64_t",
            FlagType::F64 => "double",
            FlagType::Str => "char const *",
            FlagType::Enum => "TODO",
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

fn main() {
    let cfg = std::fs::read_to_string("cfgopt.toml").unwrap();
    let app: App = toml::from_str(&cfg).unwrap();

    app.c_struct(&mut std::io::stdout()).unwrap();
    app.c_init(&mut std::io::stdout()).unwrap();
    app.c_parse(&mut std::io::stdout()).unwrap();
}
