with import <nixpkgs> {};

mkShell {
  name = "chip8-env";
  nativeBuildInputs = [
    SDL2
    cmake
    gdb
  ];
}
