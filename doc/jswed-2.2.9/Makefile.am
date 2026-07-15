
SUBDIRS=share .


bin_PROGRAMS=jswed
noinst_DATA=jswed.lyx jswed.pdf jswed.txt
EXTRA_DIST=$(srcdir)/config/* COPYING copying.txt INSTALL.txt README \
	    AUTHORS ChangeLog \
	    Install/JSWED.bmp Install/JSWED_L.bmp Install/jswed.iwz \
	    $(noinst_DATA)

jswed_SOURCES = \
	aboutbox.cxx \
	bitmap.cxx	    bitmap.hxx \
	bitmapedit.cxx	    bitmapedit.hxx \
	button.cxx          button.hxx \
	cellattrform.cxx    cellattrform.hxx \
	cellform.cxx        cellform.hxx \
	clrchooser.cxx	    clrchooser.hxx \
	checkbox.cxx	    checkbox.hxx \
	configure.cxx \
	control.cxx	    control.hxx \
			    event.hxx \
			    executor.hxx \
	filelist.cxx	    filelist.hxx \
	filesel.cxx	    filesel.hxx \
	filetypelist.cxx    filetypelist.hxx \
	fixedguard.cxx	    fixedguard.hxx\
	fontedit.cxx	    fontedit.hxx \
	form.cxx            form.hxx \
	game.cxx            game.hxx \
	geoffgame.cxx	    geoffgame.hxx\
	geoffgedit.cxx	    geoffgedit.hxx\
	geoffguard.cxx	    geoffguard.hxx \
	guardedit.cxx	    guardedit.hxx \
	henryattrpage.cxx   henryattrpage.hxx \
	henrygame.cxx	    henrygame.hxx \
	henrymsg.cxx	    henrymsg.hxx \
	henryroom.cxx	    henryroom.hxx \
	henryroompage.cxx   henryroompage.hxx \
	henrypatchpage.cxx  henrypatchpage.hxx \
	hexedit.cxx	    hexedit.hxx \
	hexpatch.cxx	  \
	hexpage.cxx	    hexpage.hxx \
	hotspot.cxx         hotspot.hxx \
	hotspotmgr.cxx      hotspotmgr.hxx \
	label.cxx           label.hxx \
	letterchooser.cxx   letterchooser.hxx \
	list.cxx            list.hxx \
	j128attrpage.cxx    j128attrpage.hxx \
	j128game.cxx        j128game.hxx \
	j128gedit.cxx       j128gedit.hxx \
	j128glist.cxx       j128glist.hxx \
	j128guard.cxx       j128guard.hxx \
	j128mem.cxx	    j128mem.hxx \
	j128msg.cxx	    j128msg.hxx \
	j128patchpage.cxx   j128patchpage.hxx \
	j128roomform.cxx    j128roomform.hxx \
	j48msg.cxx          j48msg.hxx \
	j48attrpage.cxx     j48attrpage.hxx \
	j48clrsform.cxx     j48clrsform.hxx \
	j48game.cxx         j48game.hxx \
	j48patchpage.cxx    j48patchpage.hxx \
	j48room.cxx         j48room.hxx \
	j48roomdraw.cxx     j48roomdraw.hxx \
	j48roomnav.cxx	    j48roomnav.hxx \
	j48roompage.cxx     j48roompage.hxx \
	j48splpage.cxx      j48splpage.hxx \
	j48sprite.cxx       j48sprite.hxx \
	j48telpage.cxx	    j48telpage.hxx \
	j48udgsform.cxx	    j48udgsform.hxx \
	j64attrpage.cxx     j64attrpage.hxx \
	j64cellform.cxx     j64cellform.hxx \
	j64fbarrierform.cxx j64fbarrierform.hxx \
	j64game.cxx         j64game.hxx \
	j64gfxform.cxx      j64gfxform.hxx \
	j64gedit.cxx        j64gedit.hxx \
	j64gtableform.cxx   j64gtableform.hxx \
	j64guard.cxx        j64guard.hxx \
	j64guardpage.cxx    j64guardpage.hxx \
	j64patchpage.cxx    j64patchpage.hxx \
	j64itemform.cxx	    j64itemform.hxx \
	j64room.cxx         j64room.hxx \
	j64roomform.cxx     j64roomform.hxx \
	j64roompage.cxx     j64roompage.hxx \
	j64splpage.cxx      j64splpage.hxx \
	j64targetform.cxx   j64targetform.hxx \
	j64telpage.cxx      j64telpage.hxx \
	jfilelist.cxx	    jfilelist.hxx \
	jfilesel.cxx	    jfilesel.hxx \
	jnotebook.cxx	    jnotebook.hxx \
	jswdirbox.cxx	    jswdirbox.hxx \
	jswed.cxx           jswed.hxx \
	jswgame.cxx         jswgame.hxx \
	jswguardian.cxx     jswguardian.hxx \
	jswmsg.cxx          jswmsg.hxx \
	jswpatchpage.cxx    jswpatchpage.hxx \
	jswroomlist.cxx     jswroomlist.hxx \
			    jswscr.hxx \
	jumproom.cxx \
	loadfile.cxx				\
	manicattrpage.cxx   manicattrpage.hxx \
	manicfixedguard.cxx manicfixedguard.hxx \
        manicgame.cxx	    manicgame.hxx \
	manicbbgame.cxx     manicbbgame.hxx \
	manicspgame.cxx     manicspgame.hxx \
	manicgfxform.cxx    manicgfxform.hxx \
	manicguardian.cxx   manicguardian.hxx \
	manicguardpage.cxx  manicguardpage.hxx \
	manicguardedit.cxx  manicguardedit.hxx \
	manicitemform.cxx   manicitemform.hxx \
	manicmsg.cxx        manicmsg.hxx \
	manicpatchpage.cxx  manicpatchpage.hxx \
	manicportalpage.cxx manicportalpage.hxx \
	manicroom.cxx       manicroom.hxx \
	manicroomdraw.cxx   manicroomdraw.hxx \
	manicroomform.cxx   manicroomform.hxx \
	manicroomnav.cxx    manicroomnav.hxx \
	manicroompage.cxx   manicroompage.hxx \
	manicsprite.cxx     manicsprite.hxx \
	menu.cxx            menu.hxx \
	notebook.cxx        notebook.hxx \
	notepage.cxx        notepage.hxx \
			    patch.hxx \
	room.cxx            room.hxx \
	savefile.cxx \
	sdl_bitmap.cxx      sdl_bitmap.hxx \
	sdl_executor.cxx \
	sdl_event.cxx \
	sdl_video.cxx       sdl_video.hxx \
	screenedit.cxx      screenedit.hxx \
	screenshot.cxx      \
	spritepage.cxx	    spritepage.hxx \
	spriteform.cxx	    spriteform.hxx \
	sftxgame.cxx	    sftxgame.hxx \
	sftxgame128.cxx	    sftxgame128.hxx \
			    slistener.hxx \
	snapio.cxx          snapio.hxx \
	spinctrl.cxx        spinctrl.hxx \
	spritelist.cxx      spritelist.hxx \
	textedit.cxx        textedit.hxx \
	tunepage128mini.cxx tunepage128mini.hxx \
	uiutils.cxx \
	upgrade.cxx         upgrade.hxx \
	upgrade64.cxx   \
	upgradedata.cxx \
	utils.cxx \
	video.cxx           video.hxx \
	vidfont.cxx			\
	vspritelist.cxx     vspritelist.hxx \
	bottomscr.xbm  diag1.xbm  diag3.xbm  left.xbm    right.xbm   tick.xbm \
	diag0.xbm      diag2.xbm  down.xbm   pencil.xbm  shaded.xbm  up.xbm \
	maniclives.xbm sun.xbm air.xbm  stop.xbm trigger.xbm opener.xbm \
	zxfont.inc

%.pdf:	%.lyx
	lyx -e pdf $<

%.txt:	%.lyx
	lyx -e text $<

