#ifndef DROPWIDGETLISTBINDING_H
#define DROPWIDGETLISTBINDING_H

#include "../DataManagement/mapper.h"

namespace DropWidgetListBinding
{
inline bool SupportsParameterList(ToFormMapper* container)
{
    return container && container->IsStringList()
           && container->GetType().compare("Parameter") == 0;
}
}

#endif // DROPWIDGETLISTBINDING_H
