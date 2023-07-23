{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = import nixpkgs { inherit system; };
      in {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "3darr";
          version = "unstable";
          nativeBuildInputs = with pkgs; [ doxygen graphviz ];
          outputs = ["out" "doc"];
          src = ./.;
          installPhase = ''
            mkdir -p $out/bin
            cp 3darr $out/bin/3darr
            mkdir doc
            doxygen
            mkdir -p $doc/share
            cp -r doc $doc/share/doc
          '';
        };
        devShells.default = pkgs.stdenv.mkDerivation { name = "env"; };
      });
}
