use windres::Build;

fn main() {
    Build::new().compile("config.rc").unwrap();
}