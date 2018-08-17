{
  nixpkgs ? <nixpkgs>,
  pkgs ? import nixpkgs {},
  stdenv ? pkgs.stdenv
}:
stdenv.mkDerivation {
  name = "gdb_dummy";
  src = ./.;

  buildInputs = [ pkgs.cmake ];

  installPhase = ''
    mkdir -p $out/bin;
    cp gdb_dummyserver $out/bin;
  '';
}
