# run from source

- `cd src_python`
- `pip install -r requirements.txt`
- `cd src`
- `python -m happyserial`

# build

- `pip install --upgrade pip setuptools build`
- `cd src_python`
- `python -m build`

# install locally

- `cd src_python`
- `pip install .`

# uninstall locally

- `pip uninstall happyserial`

# upload to PyPI

- create an API token after logging in at https://pypi.org/ (it's a long string starting with `pypi-`)
- `pip install --upgrade twine`
- `cd src_python`
- `twine upload dist/*`
    - username: `__token__`
    - password: the entire token above, including the `pypi-` prefix
- update appears at https://pypi.org/project/happyserial/
