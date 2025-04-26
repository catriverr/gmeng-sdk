echo "[gmeng-homebrew-check] checking for a homebrew installation..."
which  -s brew;
if [[ $? != 0 ]] ; then
	echo "[gmeng-homebrew-installer] could not find homebrew, installing it...";
	/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)";
    echo "[gmeng-homebrew-installer] installation complete"
else
    echo "[gmeng-homebrew-check] homebrew is installed, continuing..."
fi
