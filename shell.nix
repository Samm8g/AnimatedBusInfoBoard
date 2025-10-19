{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [
    pkgs.cmake
    pkgs.kdePackages.full
    pkgs.gcc
    pkgs.libglvnd
    pkgs.mesa
    pkgs.pkg-config
    pkgs.ninja
    pkgs.libGL
    pkgs.libGLU
  ];

  shellHook = ''
    export QT_PLUGIN_PATH=${pkgs.kdePackages.full}/lib/qt-6/plugins
    export QML2_IMPORT_PATH=${pkgs.kdePackages.full}/lib/qt-6/qml
  '';
}
