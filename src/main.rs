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

impl App {
    fn c_struct<W: Write>(&self, w: &mut W) -> std::io::Result<()> {
        writeln!(w, "struct cfgopt_{} {{", self.name)?;
        for flag in &self.flags {
            writeln!(w, "\t{} cfgopt_{};", flag.r#type.c_type(), flag.name)?;
        }
        writeln!(w, "}};")?;
        Ok(())
    }

    fn c_init<W: Write>(&self, w: &mut W) -> std::io::Result<()> {
        writeln!(
            w,
            "void cfgopt_{name}_init(struct cfgopt_{name} *cfg);",
            name = self.name
        )?;

        writeln!(w, "#ifdef CFG_OPT_IMPL_{}", self.name)?;
        writeln!(
            w,
            "void cfgopt_{name}_init(struct cfgopt_{name} *cfg)",
            name = self.name
        )?;

        writeln!(w, "{{")?;
        for flag in &self.flags {
            write!(w, "\tcfg->cfgopt_{argname} = ", argname = flag.name,)?;

            if let Some(defval) = &flag.default {
                write!(w, "{}", defval)?
            } else {
                match flag.r#type {
                    FlagType::Bool => write!(w, "false"),
                    FlagType::I64 => write!(w, "0"),
                    FlagType::F64 => write!(w, "0.0"),
                    FlagType::Str => write!(w, "NULL"),
                }?;
            }

            writeln!(w, ";")?;
        }
        writeln!(w, "}}")?;
        writeln!(w, "#endif  // CFG_OPT_IMPL_{}", self.name)?;
        Ok(())
    }

    fn c_parse<W: Write>(&self, w: &mut W) -> std::io::Result<()> {
        writeln!(
            w,
            concat!(
                "void cfgopt_{name}_parse(\n",
                "\tstruct cfgopt_{name} *cfgopt_{name}_v,\n",
                "\tint argc,\n",
                "\tchar **argv);"
            ),
            name = self.name
        )?;

        writeln!(w, "#ifndef CFG_OPT_IMPL_{}", self.name)?;

        for flag in &self.flags {
            writeln!(
                w,
                concat!(
                    "static void cfgopt_{name}_parse_{argname}(\n",
                    "\tstruct cfgopt_{name} *cfg,\n",
                    "\tchar const *arg);",
                ),
                name = self.name,
                argname = flag.name
            )?;
        }

        writeln!(
            w,
            concat!(
                "\n\nvoid cfgopt_{name}_parse(\n",
                "\tstruct cfgopt_{name} *cfg,\n",
                "\tint argc,\n",
                "\tchar **argv)"
            ),
            name = self.name
        )?;

        writeln!(w, "{{")?;
        writeln!(w, "\tcfgopt_{name}_init(cfg);", name = self.name)?;

        writeln!(w, "\tfor (int argi = 1; argi < argc; ++argi) {{")?;
        for flag in &self.flags {
            writeln!(
                w,
                "\t\tif (strncmp(\"-{argname}\", argv[argi], {argname_len}) == 0) {{",
                argname = flag.name,
                argname_len = flag.name.len(),
            )?;
            writeln!(
                w,
                "\t\t\tcfgopt_{name}_parse_{typename}(cfg, &cfg->cfg_{flag}, argv[argi]);",
                name = self.name,
                flag = flag.name,
                typename = flag.r#type.name(),
            )?;
            writeln!(w, "\t\t\tcontinue;")?;
            writeln!(w, "\t\t}}")?;
        }
        writeln!(w, "\t}}")?;

        writeln!(w, "}}")?;

        writeln!(w)?;

        writeln!(w, "#endif  // CFG_OPT_IMPL_{}", self.name)?;

        Ok(())
    }

    fn c_code<W: Write>(&self, w: &mut W) -> std::io::Result<()> {
        writeln!(w, "#ifndef CFGOPT_{}_H_", self.name)?;
        writeln!(w, "#define CFGOPT_{}_H_", self.name)?;

        writeln!(w)?;
        writeln!(w)?;

        writeln!(w, "#include <stdint.h>")?;
        writeln!(w, "#include <string.h>")?;
        writeln!(w, "#include <stdbool.h>")?;

        writeln!(w)?;
        writeln!(w)?;

        self.c_struct(w)?;

        writeln!(w)?;
        writeln!(w)?;

        self.c_init(w)?;

        writeln!(w)?;
        writeln!(w)?;

        self.c_parse(w)?;

        writeln!(w)?;
        writeln!(w)?;

        writeln!(w, "#endif  // CFGOPT_{}_H_", self.name)?;

        Ok(())
    }
}

fn main() -> Result<()> {
    let cfg = std::fs::read_to_string("cfgopt.toml").unwrap();
    let app: App = toml::from_str(&cfg).unwrap();

    let mut fout = File::create("cfgopt_gen.h").unwrap();
    write!(&mut fout, "{}", app.render().unwrap()).unwrap();
    // app.c_code(&mut fout).unwrap();

    Ok(())
}
