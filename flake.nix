{
  description = "C++17 / OpenGL (GLFW + GLEW + GLM) development shell";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-26.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            # Toolchain & build
            cmake
            gcc
            ninja
            pkg-config

            # Editor / dev utilities (clangd comes from clang-tools)
            clang-tools
            neovim
            git
            ripgrep
            fd

            # OpenGL stack
            glfw
            glew
            glm
            libGL
            libx11
            xorgproto
          ];

          # clangd on NixOS needs to query the nix store gcc for builtin
          # include paths; the LazyVim clangd extra ships the right
          # --query-driver flag, this just documents why gcc is here.
          shellHook = ''
            echo "OpenGL dev shell (C++17, GLFW/GLEW/GLM)"
            echo "  cmake -S . -B build -G Ninja && cmake --build build"
            echo "  ln -sfn build/compile_commands.json compile_commands.json  # once, for clangd"
          '';
        };
      });
}
