#temporary till KDE4 and it can be renamed
xdg_apps_DATA = defaultapplication.desktop

xdg_menu_DATA = system-preferences.menu system-preferences.directory

xdg_mergedmenu_DATA = system-preferences-merge.menu
xdg_mergedmenudir = $(xdg_menudir)/applications-merged

xdg_directory_DATA = \
kde-settings-appearance.directory \
kde-settings-panel.directory \
kde-settings-useraccount.directory \
kde-settings-system.directory

install-data-local:
	for file in `cat $(srcdir)/old_desktop_files` ; do \
	    if test -f $(DESTDIR)$(xdg_appsdir)/$${file} ; then \
	        rm $(DESTDIR)$(xdg_appsdir)/$${file} ; \
		echo Deleting $(DESTDIR)$(xdg_appsdir)/$${file} ; \
	    fi; \
	done;
