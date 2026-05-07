import { app, BrowserWindow, ipcMain } from 'electron'
import started from 'electron-squirrel-startup'
import { ipcChannels } from './main/bridge/ipcChannels'
import { listSerialPorts } from './main/serial/serialScanner'
import { SerialService } from './main/serial/serialService'
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
	ipcMain.handle(ipcChannels.appSetLocale, (_event, locale: string) => {
		setTrayLocale(normalizeLocale(locale))
		refreshAppTray({
			showMainWindow,
			serialService,
			onQuit: () => {
				isQuitting = true
			},
		})
	})
}

function showMainWindow(): void {
	mainWindow?.show()
	mainWindow?.focus()
}

function createWindows(): void {
	mainWindow = createMainWindow()

	mainWindow.on('close', (event) => {
		if (isQuitting) {
			return
		}
		event.preventDefault()
		mainWindow?.hide()
	})

	createAppTray({
		showMainWindow,
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
