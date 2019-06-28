#include <Windows.h>
#include <CommCtrl.h>

void addColumn(HWND hwMem, const char text[], int width, int col) {
	LVCOLUMN lvc;
	char* tp = (char *)text;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = width;
	lvc.pszText = tp;
	lvc.iSubItem = col;
	ListView_InsertColumn(hwMem, col, &lvc);
}

void addRow(HWND hwMem, const char text[])
{
	LVITEM lvi = { 0 };

	lvi.mask = LVIF_TEXT;
	lvi.pszText = (char *)text;
	ListView_InsertItem(hwMem, &lvi);
}