# Autonomous Selector Integration Notes

## Added files
- `include/auton_selector.h`
- `src/auton_selector.cpp`
- `browser_demo/index.html`

## Updated file
- `src/main.cpp`

## Behavior
- Left side of the V5 Brain screen shows the current robot pose.
- Right side shows a scrollable autonomous list built from calls like:

```cpp
selector.add(myAutoFunction, "My Auto Name");
```

- Pressing **Confirm**:
  - locks the selected autonomous routine,
  - disables the confirm button,
  - disables the list,
  - shows `Running autonomous [function name]` below the button.

## Integration
The selector is separate from your motion code and autonomous routine definitions.

`initialize()` now registers autos and starts the UI.

`autonomous()` now runs:

```cpp
selector.runSelected();
```

## Browser demo
Open `browser_demo/index.html` in any browser.

That demo mirrors the selector UI behavior:
- scroll the list,
- choose a routine,
- press confirm,
- see the lock-in state and disabled button.

## Fallback behavior
If you never press confirm, `runSelected()` falls back to the currently highlighted autonomous.
