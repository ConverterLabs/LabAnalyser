# Crash- and null-safety hardening (package A)

## Approved scope and completed guards

Package A applies only narrow no-op guards at previously unsafe null, absent-ID
and invalid-selection boundaries. Public APIs, Qt signals/slots, normal signal
order, plugin IID and `InterfaceData` ABI are unchanged.

- `DM_SAFE_001`: direct `DataManagementSetClass::SendNewValue()` and
  parameterless `UpdateRequest()` without a sender, and connected unknown
  widgets, perform no lookup insertion, mutation or `MessageSender` emission.
  `SetData()` with empty or missing IDs remains a no-op.
- `DM_SAFE_002`: parentless/direct `MessengerClass` instances no longer
  dereference absent parent chains. `CloseProject` still emits its close signal
  but omits the notification when no application origin exists; the normal
  `LabAnalyser -> manager -> messenger` order and notification are unchanged.
  `NewDeviceRegistration(nullptr)`, `SendInfo` and `SendError` without a
  parent are no-ops.
- `DM_SAFE_003`: null QObject arguments cannot create bindings or be
  dereferenced by DataManagement lookup, type, interface or removal helpers.
  `SetData()` ignores stored null form pointers.
- `GUI_SAFE_001`: direct senderless `RemoveDevice` and
  `dockWidget_topLevelChanged`, plus empty/top-level `ChangeMinMaxValue`
  selection, leave manager/tree state unchanged and do not open a dialog.

## Evidence and exclusions

Focused `DataManagementCharacterizationTests` was rebuilt incrementally after
each DataManagement slice and passed (exit code 0). The real offscreen
`MainWindowIntegrationTests` rebuilt in its existing tree after one external
120-second continuation and passed (exit code 0). The continuation was not a
compiler or test failure.

The one planned non-clean central-runner checkpoint was resumed once in the
same build tree after its first external 120-second limit. Both invocations
completed the plugin prebuild and the early unit/DataManagement targets without
a compiler or test failure, but the second reached the same external limit
while rebuilding the XML application graph. It is therefore **not** reported
as a green completed runner; GitHub CI remains the pending package checkpoint.

`WidgetBindingRegistry` intentionally remains object-name keyed and has no
QObject destruction connection. A nonnull stale raw `FormP` cannot be safely
distinguished from a live QObject without an ownership or binding-model change;
it is therefore not dereferenced by new tests and remains a Milestone-5
ownership risk. This package does not migrate renamed bindings, change the
public mutable container map, alter regular Messenger signal order or add
visible error messages.
