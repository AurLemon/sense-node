import { app, BrowserWindow, ipcMain } from 'electron'
import started from 'electron-squirrel-startup'
import { ipcChannels } from './main/bridge/ipcChannels'
import { listSerialPorts } from './main/serial/serialScanner'
import { SerialService } from './main/serial/serialService'
import { createHudWindow } from './main/windows/hudWindow'
import { createMainWindow } from './main/windows/mainWindow'
import { createAppTray } from './main/windows/tray'

if (started) {
	app.quit()
}

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
		mainWindow?.show()
		mainWindow?.focus()
		hudWindow?.hide()
	})
	ipcMain.handle(ipcChannels.appShowHud, () => hudWindow?.showInactive())
	ipcMain.handle(ipcChannels.appHideHud, () => hudWindow?.hide())
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
		hudWindow?.showInactive()
	})

	mainWindow.on('minimize', () => {
		hudWindow?.showInactive()
	})

	hudWindow.on('closed', () => {
		hudWindow = null
	})

	createAppTray({
		mainWindow,
		hudWindow,
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
	mainWindow.show()
})
