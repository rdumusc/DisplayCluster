class pDisplayGroupPython
{
%TypeHeaderCode
#include <DisplayGroupPython.h>
%End
};

class pyDisplayGroupPython
{
%TypeHeaderCode
#include <DisplayGroupPython.h>
%End

  public:
    pyDisplayGroupPython();

    pDisplayGroupPython get();

    void addContentWindow(const pyContentWindow &);
    void removeContentWindow(const pyContentWindow &);
    void moveContentWindowToFront(const pyContentWindow &);
    void saveState(const char * filename);
    void loadState(const char * filename);

    int getNumContentWindows();
    pyContentWindow getPyContentWindow(int);
};
