{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [
    pkgs.cmake
    pkgs.qt6.qtbase
    pkgs.gcc
    pkgs.libglvnd
    pkgs.mesa
    pkgs.pkg-config
    pkgs.ninja
  ];
}
