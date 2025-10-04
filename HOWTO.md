# Delta Station How-To

Delta Station was designed to be configurable by anyone with any skill-set. The goal of this document is for you to
learn the following:
- how to interpret Delta Station,
- how to configure it to display a custom live graph, and
- how to modify the buffer layout.

Let's get started.

## Interpreting Delta Station

Delta Station's design philosophy is to be a highly customizable packet-to-graph and packet-to-database translator (with
a small amount of strategy sending to the car). This is achieved by writing all configuration in a separate file called
`config.toml`. This file is written in [TOML](https://en.wikipedia.org/wiki/TOML) (Tom's Obvious, Minimal Language).
Here is a TOML snippet showing some of its basics:
```toml
# Comments are made with a hashtag symbol.

# You can define fields like this:
title = "demo toml file"

# Fields can have children (called tables) if you declare it with [brackets].
# What immediately follows are children of this table...
[header1]
property1 = true
property2 = [5, false, -1]

# This is a separate table, not a child of header1.
[header2]
property1 = false # This is an end-of-line comment.

# This is how you declare a child that can have children.
[header2.subheader1]
# Values can themselves be tables.
property0 = {inner0 = false, inner1 = true}
```

The configuration file we use shall have the following structure:

```toml
# The top of the file shall have this table declaration.
[ds]
dynamic = true

# There shall be a list of buffers.
[ds.buffer0]
id = 0x1 # This is the id number received from Telemetry.
# Here, `buffer0.foo` and `buffer0.bar` are the names used by the graph subsystem.
# `name` is used for displaying this field. It is optional; replace with the empty string "" if it's not needed.
# `type` is the expected datatype. this is its format:
#   first character, one of {f,i,u}: determines if data type is floating point, signed, or unsigned respectively.
#   next two characters, one of {8,16,32,64}: used for the width of the data type (in bits).
#   in this example, the type would correspond to a C/C++ "double". Note: f8 and f16 are invalid!
# `order` determines where in the buffer it lives.
foo = {name = "Field Display Name", type = "f64", order = 0}
bar = {name = "Field Display Name", type = "u32", order = 1}

# There shall be a logger configuration. It is currently unused.
[logger]
enabled = true

# There may be a list of custom graphs.
[graph]
# `expr` is the formula evaluated at each frame.
# `length` is the amount of historic data displayed on the graph, measured in seconds.
# `type` is currently unused; will be extended to include "line", "bar", "histogram", etc.
graph_name = {expr = "(buffer0.foo * buffer0.bar) / 2.0", length = 10.0, type = "normal"}
```

There is a [default config file](sample_config.toml) if you'd like to reference it. The file must be in the same
directory as the command line from which you're running Delta Station in.
