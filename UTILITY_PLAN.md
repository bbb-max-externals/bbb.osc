# bbb.osc.path Utility Plan

## Overview

`bbb.osc.path` is a set of Max externals for fast OSC selector/address-path manipulation. It is intended to complement `bbb.osc` UDP send/receive externals, especially in high-throughput OSC patches where Max `js`-based path handling can become a bottleneck.

The utilities should operate on Max messages where the first atom is an OSC path/selector and the remaining atoms are payload, unless the specific object is designed to transform multiple leading path components.

Example message shape:

```max
/hoge/foo/bar 1 2 3
```

- OSC path: `/hoge/foo/bar`
- Payload: `1 2 3`

Default behavior should preserve the payload unless explicitly disabled.

---

## Design Goals

1. **Low latency under high message rates**
   - Avoid `js` object overhead for repetitive OSC address operations.
   - Minimize allocation and symbol creation per message.

2. **OSC-specific semantics**
   - Treat slash-delimited address paths as first-class structures.
   - Support common namespace operations such as append, prepend, strip, route, match, head/tail extraction, and join/unjoin.

3. **Max-friendly object design**
   - Provide small, readable utility objects.
   - Also provide a combined `bbb.osc.path` object in the style of `zl`, where the first argument selects the operation.

4. **Predictable payload behavior**
   - Default: payload is passed through.
   - Optional: output only the transformed path.

5. **Configurable normalization**
   - Handle inconsistent slash usage where useful.
   - Allow strict mode when the patcher needs exact string behavior.

---

## Core Objects

### `bbb.osc.path.join`

Joins multiple leading OSC path components into a single OSC address path.

Example:

```max
/hoge /foo /bar 1 2 3
→ /hoge/foo/bar 1 2 3
```

#### Endpoint / component-count handling

This object needs explicit rules for deciding which leading atoms are path components and where the payload begins.

Default rule:

- Starting from atom 0, consume atoms that are symbols and are path-like.
- A path-like atom is a symbol beginning with `/`, unless relaxed mode is enabled.
- Stop at the first non-path-like atom.
- The remaining atoms are treated as payload.

Example:

```max
/hoge /foo abc 1 2
→ /hoge/foo abc 1 2
```

Here, `abc` is payload because it does not begin with `/`.

#### Proposed attributes

```max
@count 0
```

Number of leading atoms to join.

- `0`: auto-detect endpoint using the default rule.
- `N > 0`: join exactly `N` leading atoms if available.

Examples:

```max
bbb.osc.path.join @count 3
/hoge /foo /bar 1 2 3
→ /hoge/foo/bar 1 2 3
```

```max
bbb.osc.path.join @count 2
/hoge /foo /bar 1 2 3
→ /hoge/foo /bar 1 2 3
```

Potential additional option:

```max
@maxcount 0
```

Maximum number of leading path-like atoms to join.

- `0`: no maximum.
- `N > 0`: auto-detect, but consume at most `N` atoms.

This may be more convenient than `@count` when input can contain variable-depth OSC paths but should be capped.

---

### `bbb.osc.path.unjoin`

Splits a single OSC address path into multiple path component atoms.

Example:

```max
/hoge/foo/bar 1 2 3
→ /hoge /foo /bar 1 2 3
```

#### Attributes

```max
@slash 1
```

Controls whether split components include a leading slash.

```max
@slash 1
/hoge/foo/bar 1 2 3
→ /hoge /foo /bar 1 2 3
```

```max
@slash 0
/hoge/foo/bar 1 2 3
→ hoge foo bar 1 2 3
```

```max
@payload 1
```

Preserve payload after split path components. Default: `1`.

---

### `bbb.osc.path.append`

Appends one or more components to the incoming OSC path.

Example:

```max
bbb.osc.path.append /path
/hoge 1 2 3
→ /hoge/path 1 2 3
```

The object should accept argument components with or without slashes, depending on normalization settings.

Examples with normalization enabled:

```max
bbb.osc.path.append path
bbb.osc.path.append /path
bbb.osc.path.append /path/
```

All should produce:

```max
/hoge/path
```

---

### `bbb.osc.path.prepend`

Prepends one or more components to the incoming OSC path.

Example:

```max
bbb.osc.path.prepend /path
/hoge 1 2 3
→ /path/hoge 1 2 3
```

The originally considered name `repend` was a typo. Use `prepend`.

---

### `bbb.osc.path.removehead`

Removes the first `N` components from the OSC path.

Example:

```max
bbb.osc.path.removehead 2
/hoge/foo/bar 1 2 3
→ /bar 1 2 3
```

If `N` is greater than or equal to the path depth, define a predictable result. Recommended behavior:

```max
/hoge/foo 1 2 3
bbb.osc.path.removehead 2
→ / 1 2 3
```

Alternatively, this may output an empty symbol or route to the no-match outlet. `/` is probably safer and more OSC-like.

---

### `bbb.osc.path.removetail`

Removes the last `N` components from the OSC path.

Example:

```max
bbb.osc.path.removetail 2
/hoge/foo/bar 1 2 3
→ /hoge 1 2 3
```

If `N` is greater than or equal to the path depth, recommended output is `/` plus payload.

---

## Additional Recommended Objects

### `bbb.osc.path.head`

Keeps the first `N` components of the OSC path.

Example:

```max
bbb.osc.path.head 2
/hoge/foo/bar 1 2 3
→ /hoge/foo 1 2 3
```

---

### `bbb.osc.path.tail`

Keeps the last `N` components of the OSC path.

Example:

```max
bbb.osc.path.tail 2
/hoge/foo/bar/baz 1 2 3
→ /bar/baz 1 2 3
```

---

### `bbb.osc.path.nth`

Extracts the `N`th component from the OSC path.

Example:

```max
bbb.osc.path.nth 1
/hoge/foo/bar 1 2 3
→ /foo 1 2 3
```

Possible attribute:

```max
@payload 1
```

- `1`: output selected component plus original payload.
- `0`: output selected component only.

Indexing should be zero-based by default because this is more natural in C/C++ implementation and common programming practice.

---

### `bbb.osc.path.replace`

Replaces one component or a matching component/prefix in the OSC path.

Possible forms:

```max
bbb.osc.path.replace 0 /device
/hoge/foo/bar 1 2 3
→ /device/foo/bar 1 2 3
```

```max
bbb.osc.path.replace /hoge /device
/hoge/foo/bar 1 2 3
→ /device/foo/bar 1 2 3
```

This is useful for OSC proxying, bridge patches, namespace remapping, and device ID rewriting.

---

### `bbb.osc.path.strip`

Removes a prefix if the OSC path begins with that prefix.

Example:

```max
bbb.osc.path.strip /device/1
/device/1/param/freq 440
→ /param/freq 440
```

If the prefix does not match, behavior should be controlled by no-match handling.

Recommended default:

- Match: left outlet outputs stripped message.
- No match: rightmost outlet outputs original message.

---

### `bbb.osc.path.prefix`

Tests whether a path has the specified prefix, optionally passing only matched messages.

Example:

```max
bbb.osc.path.prefix /device/1
/device/1/freq 440
→ /device/1/freq 440
```

```max
bbb.osc.path.prefix /device/1
/device/2/freq 440
→ rightmost outlet: /device/2/freq 440
```

This acts as an OSC-specific filter and is useful before deeper routing.

---

### `bbb.osc.path.match`

Matches an OSC path against a pattern.

Example:

```max
bbb.osc.path.match /synth/*/freq
/synth/1/freq 440
→ match outlet
```

```max
/synth/1/gain 0.5
→ no-match outlet
```

#### Pattern support

OSC address pattern syntax to consider:

```text
*        wildcard, zero or more chars inside a component or across path text depending on mode
?        single-character wildcard
[abc]    character set
[!abc]   negated character set
{foo,bar} alternatives
```

Implementation may start with `*` only, then expand later.

Important: pattern strings should be parsed/compiled when arguments or attributes change, not on every incoming message.

---

### `bbb.osc.path.route`

Routes messages by exact OSC path or path pattern.

Example:

```max
bbb.osc.path.route /foo /bar /baz
```

```max
/foo 1 2 3
→ outlet 0: 1 2 3

/bar 4 5 6
→ outlet 1: 4 5 6

/qux 7 8 9
→ rightmost outlet: /qux 7 8 9
```

This should behave like a Max-native OSC path router.

Possible modes:

```max
@mode exact
@mode prefix
@mode pattern
```

Payload stripping behavior should be considered carefully:

- For `@mode exact`, outputting payload only matches Max `route` convention.
- For `@mode prefix`, it may be useful to output the unmatched suffix path plus payload.
- Add an explicit attribute if necessary:

```max
@strip 1
```

Example:

```max
bbb.osc.path.route /foo @mode prefix @strip 1
/foo/bar/baz 1 2 3
→ /bar/baz 1 2 3
```

---

### `bbb.osc.path.dispatch`

A higher-level hierarchical dispatcher for OSC namespaces.

Example:

```max
bbb.osc.path.dispatch /foo /bar /hoge/*
```

Useful for routing large OSC namespaces into subpatches or modules.

Recommended attributes:

```max
@mode exact|prefix|pattern
@strip 0|1
```

Example:

```max
bbb.osc.path.dispatch /foo @mode prefix @strip 1
/foo/bar/baz 1 2 3
→ /bar/baz 1 2 3
```

`dispatch` may overlap with `route`. If implementation complexity should be minimized, implement `route` first and postpone `dispatch`.

---

## Combined Object: `bbb.osc.path`

Provide a combined utility object similar to Max `zl`, where the first argument selects the operation.

Examples:

```max
bbb.osc.path join
bbb.osc.path unjoin
bbb.osc.path append /foo
bbb.osc.path prepend /foo
bbb.osc.path removehead 2
bbb.osc.path removetail 2
bbb.osc.path head 2
bbb.osc.path tail 2
bbb.osc.path nth 1
bbb.osc.path strip /device/1
bbb.osc.path prefix /device/1
bbb.osc.path match /synth/*/freq
bbb.osc.path route /foo /bar /baz
```

This object is useful for compact patching and documentation, while separate class names remain preferable when readability is important.

Recommended implementation strategy:

- Implement core logic in shared C/C++ utility functions.
- Expose thin Max classes for each named object.
- Expose `bbb.osc.path` as a dispatcher over the same internal implementation.

---

## Shared Attributes

### `@normalize`

Controls slash/path normalization.

Default recommendation: `1`.

When enabled, normalize paths and argument components as follows:

```text
foo/bar      → /foo/bar
/foo/bar     → /foo/bar
/foo/bar/    → /foo/bar
//foo///bar/ → /foo/bar
```

For append/prepend, this means the following should be equivalent:

```max
bbb.osc.path.append path
bbb.osc.path.append /path
bbb.osc.path.append /path/
```

All produce:

```max
/hoge/path
```

#### Is slash handling part of `@normalize`?

Recommended: yes, initially.

Rationale:

- Users generally expect OSC paths to be canonicalized as `/a/b/c`.
- Append/prepend slash absorption is a form of path normalization.
- Fewer attributes make the API easier to learn.

Potential future split if needed:

```max
@normalize 1
@strictslash 0
```

or:

```max
@autoslash 1
```

But avoid adding this unless strict slash behavior becomes necessary in real patches.

---

### `@payload`

Controls whether the original payload is preserved.

Default: `1`.

```max
@payload 1
/path 1 2 3
→ /new/path 1 2 3
```

```max
@payload 0
/path 1 2 3
→ /new/path
```

This should apply to path-transforming objects where payload preservation is meaningful:

- `join`
- `unjoin`
- `append`
- `prepend`
- `removehead`
- `removetail`
- `head`
- `tail`
- `nth`
- `replace`
- `strip`

For `route`, payload behavior may need a separate convention because Max `route` usually removes the selector.

---

### `@slash`

Primarily for `unjoin` and possibly `nth`.

Default: `1`.

```max
@slash 1
/hoge/foo/bar
→ /hoge /foo /bar
```

```max
@slash 0
/hoge/foo/bar
→ hoge foo bar
```

---

### `@mode`

For match/filter/route objects.

Recommended values:

```text
exact
prefix
pattern
```

#### `exact`

The full path must match.

```max
@mode exact
/foo     → match
/foo/bar → no match
```

#### `prefix`

The path must begin with the specified prefix at a component boundary.

```max
@mode prefix
/foo     → match
/foo/bar → match
/foobar  → no match
```

Important: prefix matching should respect component boundaries. `/foo` should not match `/foobar`.

#### `pattern`

Use OSC address pattern matching.

---

### `@nomatch`

Initial recommendation: do not expose this as a user-facing attribute unless needed.

Max-friendly default:

- Matched result goes from the relevant left/middle outlet.
- Non-matched messages go from the rightmost outlet.

This is consistent with Max routing idioms and keeps patching explicit.

If later needed, possible values:

```text
outlet
pass
ignore
```

But for now, rightmost outlet behavior is enough.

---

## Outlet Conventions

For pure transform objects:

```text
outlet 0: transformed message
```

For match/filter objects:

```text
outlet 0: matched message or transformed matched message
outlet 1: non-matched original message
```

For route-like objects with N route entries:

```text
outlet 0..N-1: matched output for corresponding route entry
outlet N: unmatched original message
```

The unmatched outlet should be the rightmost outlet.

---

## Path Normalization Details

Recommended canonical form:

```text
/foo/bar
```

Rules:

1. Path begins with a single `/`.
2. Multiple consecutive slashes collapse to one slash.
3. Trailing slash is removed, except for root `/`.
4. Empty path becomes `/`.
5. Component boundary logic should operate after normalization when `@normalize 1`.

Examples:

```text
foo       → /foo
/foo      → /foo
/foo/     → /foo
//foo     → /foo
/foo//bar → /foo/bar
/         → /
```

Strict mode with `@normalize 0` should avoid changing the input except where the operation intrinsically requires joining components.

---

## Edge Cases

### Empty or invalid input

Input with no atoms:

```max
bang
```

Potential behavior:

- Transform objects: output `/` or ignore.
- Route/match objects: send to unmatched outlet or ignore.

Recommended: ignore `bang` unless a clear useful behavior exists.

### First atom is not a symbol

Example:

```max
1 2 3
```

Recommended:

- Transform objects: rightmost/error outlet if available, otherwise ignore.
- Route/match objects: unmatched outlet.

### Root path

```max
/ 1 2 3
```

Operations should behave predictably:

```text
append /foo     → /foo 1 2 3
prepend /foo    → /foo 1 2 3
removehead 1    → / 1 2 3
removetail 1    → / 1 2 3
unjoin @slash 1 → / 1 2 3, or just / followed by payload
```

### Component count exceeds depth

Recommended output for removal operations: root `/` plus payload.

```max
bbb.osc.path.removehead 99
/foo/bar 1 2 3
→ / 1 2 3
```

---

## Implementation Notes

### Language / structure

Use C or C++ Max SDK externals.

Recommended architecture:

1. Shared OSC path utility module.
2. Thin wrapper class per object.
3. Shared parser/normalizer/matcher functions.
4. Optional combined `bbb.osc.path` dispatcher object.

Suggested internal functions:

```c
normalize_path(...)
split_path_components(...)
join_path_components(...)
append_path(...)
prepend_path(...)
remove_head_components(...)
remove_tail_components(...)
match_path_exact(...)
match_path_prefix(...)
compile_osc_pattern(...)
match_osc_pattern(...)
```

### Allocation strategy

For high-throughput operation:

- Reuse output atom buffers where possible.
- Avoid heap allocation per message for common cases.
- Use stack/local fixed buffers for short paths if safe.
- Fall back to dynamic allocation for long paths.
- Cache normalized argument paths/components.
- Cache compiled patterns.
- Avoid repeated `gensym()` calls for identical output paths where possible, though final Max symbols will still require `gensym`.

### Symbol handling

Max messages represent OSC paths as symbols, so final output path components need to be symbols.

Optimization considerations:

- Parse incoming `t_symbol->s_name` directly.
- Do not copy the path string unless the operation requires modification.
- For exact/prefix matching, compare string slices where possible.
- Normalize only when `@normalize 1` or when operation requires canonical output.

### Pattern matching

For `match`, `route @mode pattern`, and `dispatch`:

- Compile patterns at object creation or when arguments/attributes change.
- Store compiled representation in the object instance.
- Avoid parsing pattern syntax per incoming message.
- Start with a small supported subset if needed, preferably `*` first.
- Add `?`, character classes, and `{a,b}` alternatives later.

### Route performance

For `bbb.osc.path.route`:

- Exact mode can use a hash table from normalized path string to outlet index.
- Prefix mode can use sorted prefixes or a trie if route lists become large.
- Pattern mode likely requires linear scan over compiled patterns unless a more complex index is needed.
- Preserve declaration order when multiple route entries could match.

Recommended matching priority:

1. First matching route argument wins.
2. Output from the outlet corresponding to that route argument.
3. If no match, output from the rightmost outlet.

### Component boundary correctness

Prefix matching must avoid false positives:

```text
prefix /foo should match /foo and /foo/bar
prefix /foo should not match /foobar
```

Implementation rule:

- Exact string match is valid.
- Otherwise, path must begin with prefix and the next character after prefix must be `/`.

### Threading / scheduler considerations

These objects should be safe for normal Max message-thread use.

If used directly after UDP receive objects, avoid expensive work in time-critical contexts. Keep operations deterministic and bounded where possible.

Avoid blocking, file I/O, logging spam, or dynamic pattern recompilation on hot message paths.

---

## Initial Implementation Priority

Recommended order:

1. `bbb.osc.path.join`
2. `bbb.osc.path.unjoin`
3. `bbb.osc.path.append`
4. `bbb.osc.path.prepend`
5. `bbb.osc.path.removehead`
6. `bbb.osc.path.removetail`
7. `bbb.osc.path.head`
8. `bbb.osc.path.tail`
9. `bbb.osc.path.strip`
10. `bbb.osc.path.prefix`
11. `bbb.osc.path.route`
12. `bbb.osc.path.match`
13. `bbb.osc.path.nth`
14. `bbb.osc.path.replace`
15. Combined `bbb.osc.path` dispatcher, if not built from the start

A practical first release could include:

```text
join
unjoin
append
prepend
removehead
removetail
strip
prefix
route
```

This would already cover most OSC namespace manipulation use cases.

---

## Naming Summary

Use:

```text
bbb.osc.path.join
bbb.osc.path.unjoin
bbb.osc.path.append
bbb.osc.path.prepend
bbb.osc.path.removehead
bbb.osc.path.removetail
bbb.osc.path.head
bbb.osc.path.tail
bbb.osc.path.nth
bbb.osc.path.replace
bbb.osc.path.strip
bbb.osc.path.prefix
bbb.osc.path.match
bbb.osc.path.route
bbb.osc.path.dispatch
bbb.osc.path
```

Do not use:

```text
bbb.osc.path.repend
```

`repend` was a typo; the correct name is `prepend`.

---

## Open Questions

1. Should `join` support both `@count` and `@maxcount`, or is one enough?
2. Should append/prepend slash absorption be fully covered by `@normalize`, or should there eventually be a separate `@autoslash` / `@strictslash` attribute?
3. Should `route` default to outputting payload only, like Max `route`, or output the full original message?
4. Should root `/` be the universal fallback for empty paths after removal, or should an empty symbol be allowed in strict mode?
5. How much OSC pattern syntax should be included in the first release?

Recommended initial decisions:

```text
@normalize covers slash absorption.
@payload defaults to 1 for transform objects.
No-match goes to the rightmost outlet.
removehead/removetail beyond depth output root `/` plus payload.
join supports @count first; @maxcount can be added if needed.
match supports exact, prefix, and initially simple `*` pattern matching.
```
