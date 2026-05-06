import { Menu, Tray, app, nativeImage } from 'electron'
import type { BrowserWindow } from 'electron'
import type { SerialService } from '../serial/serialService'

export function createAppTray(args: {
	mainWindow: BrowserWindow
	hudWindow: BrowserWindow
	serialService: SerialService
	onQuit: () => void
}): Tray {
	const image = nativeImage.createEmpty()
	const tray = new Tray(image)
	tray.setToolTip('SenseNode')

	const refreshMenu = () => {
		const menu = Menu.buildFromTemplate([
			{
				label: 'Show Main Window',
				click: () => {
					args.mainWindow.show()
					args.mainWindow.focus()
					args.hudWindow.hide()
				},
			},
			{ label: 'Hide HUD', click: () => args.hudWindow.hide() },
			{ label: 'Show HUD', click: () => args.hudWindow.showInactive() },
			{ type: 'separator' },
			{ label: 'Pause Serial', click: () => args.serialService.pause() },
			{ label: 'Resume Serial', click: () => args.serialService.resume() },
			{ type: 'separator' },
			{
				label: 'Quit',
				click: () => {
					args.onQuit()
					app.quit()
				},
			},
		])
		tray.setContextMenu(menu)
	}

	tray.on('click', () => {
		args.mainWindow.show()
		args.mainWindow.focus()
		args.hudWindow.hide()
	})
	refreshMenu()
	return tray
}
