// The real loader contains one PlotWidget branch.  Phase 3H.2 explicitly
// excludes plots, while the compiler still needs its constructor symbol for
// DropWidgetsUiLoader::createWidget.  These no-op symbols are link seams only;
// tests never request class "PlotWidget" and no PlotWidget object is created.
extern "C" void plotWidgetCtorC1(void*, void*, void*, bool) __asm__("_ZN10PlotWidgetC1EP10MainWindowP7QWidgetP10QStatusBarb");
extern "C" void plotWidgetCtorC2(void*, void*, void*, bool) __asm__("_ZN10PlotWidgetC2EP10MainWindowP7QWidgetP10QStatusBarb");
void plotWidgetCtorC1(void*, void*, void*, bool) {}
void plotWidgetCtorC2(void*, void*, void*, bool) {}
