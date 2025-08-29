JSMin / QMLMin
===================

Minify Javascript and QML files!

The minifying is done with Douglas Crockford's JSMin algorithm (http://www.crockford.com/javascript/jsmin.html), which is very safe and works well with QML files.

Usage
-----

```
./jsmin (options) (input file 1) [(...) (input file n)]

Options:
  -o : Output file or output directory depending on the input.
```

Building
-----

```
make
```

Install
-----

```
make install PREFIX=(prefix)
```

Uninstall
-----

```
make uninstall PREFIX=(prefix)
```