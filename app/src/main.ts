import { app, BrowserWindow, ipcMain } from 'electron'
import started from 'electron-squirrel-startup'
import { ipcChannels } from './main/bridge/ipcChannels'
import { listSerialPorts } from './main/serial/serialScanner'
import { SerialService } from './main/serial/serialService'
import { createHudWindow } from './main/windows/hudWindow'
import { createMainWindow } from './main/windows/mainWindow'
import {
	createAppTray,
	setTrayLocale,
	refreshAppTray,
} from './main/windows/tray'
import { normalizeLocale } from './shared/i18n'

if (started) {
	app.quit()
}

app.setAppUserModelId('SenseNode')

let mainWindow: BrowserWindow | null = null
let hudWindow: BrowserWindow | null = null
let isQuitting = false
const serialService = new SerialService()

function broadcast(channel: string, payload: unknown): void {
	for (const window of BrowserWindow.getAllWindows()) {
		window.webContents.send(channel, payload)
	}
}

function registerIpc(): void {
	ipcMain.handle(ipcChannels.serialListPorts, () => listSerialPorts())
	ipcMain.handle(
		ipcChannels.serialConnect,
		async (_event, path: string, baudRate?: number) => {
			await serialService.connect(path, baudRate)
			return serialService.getStatus('connected')
		},
	)
	ipcMain.handle(ipcChannels.serialDisconnect, async () => {
		await serialService.disconnect()
		return serialService.getStatus('disconnected')
	})
	ipcMain.handle(ipcChannels.serialPause, () => {
		serialService.pause()
		return serialService.getStatus('connected')
	})
	ipcMain.handle(ipcChannels.serialResume, () => {
		serialService.resume()
		return serialService.getStatus('connected')
	})
	ipcMain.handle(ipcChannels.appShowMainWindow, () => {
		showMainWindow()
	})
	ipcMain.handle(ipcChannels.appShowHud, () => showHudWindow())
	ipcMain.handle(ipcChannels.appHideHud, () => hideHudWindow())
	ipcMain.handle(ipcChannels.appSetLocale, (_event, locale: string) => {
		setTrayLocale(normalizeLocale(locale))
		refreshAppTray({
			showMainWindow,
			showHudWindow,
			hideHudWindow,
			serialService,
			onQuit: () => {
				isQuitting = true
			},
		})
	})
}

function hideHudWindow(): void {
	if (hudWindow && !hudWindow.isDestroyed()) {
		hudWindow.hide()
	}
}

function showHudWindow(): void {
	if (!hudWindow || hudWindow.isDestroyed()) {
		hudWindow = createHudWindow()
		hudWindow.on('close', (event) => {
			if (isQuitting) {
				return
			}
			event.preventDefault()
			hideHudWindow()
		})
		hudWindow.on('closed', () => {
			hudWindow = null
		})
	}

	hudWindow.showInactive()
}

function showMainWindow(): void {
	mainWindow?.show()
	mainWindow?.focus()
	hideHudWindow()
}

function createWindows(): void {
	mainWindow = createMainWindow()
	hudWindow = createHudWindow()

	mainWindow.on('close', (event) => {
		if (isQuitting) {
			return
		}
		event.preventDefault()
		mainWindow?.hide()
		showHudWindow()
	})

	mainWindow.on('minimize', () => {
		showHudWindow()
	})

	hudWindow.on('close', (event) => {
		if (isQuitting) {
			return
		}
		event.preventDefault()
		hideHudWindow()
	})

	createAppTray({
		showMainWindow,
		showHudWindow,
		hideHudWindow,
		serialService,
		onQuit: () => {
			isQuitting = true
		},
	})
}

serialService.on('frame', (frame) => broadcast(ipcChannels.deviceFrame, frame))
serialService.on('line', (line) => broadcast(ipcChannels.serialLine, line))
serialService.on('status', (status) =>
	broadcast(ipcChannels.dataSourceStatus, status),
)

app.on('ready', () => {
	registerIpc()
	createWindows()
})

app.on('before-quit', () => {
	isQuitting = true
})

app.on('window-all-closed', () => {
	if (process.platform !== 'darwin') {
		app.quit()
	}
})

app.on('activate', () => {
	if (!mainWindow) {
		createWindows()
		return
	}
	showMainWindow()
})
