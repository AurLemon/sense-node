import { app, BrowserWindow, Menu, ipcMain } from 'electron'
import started from 'electron-squirrel-startup'
import { ipcChannels } from './main/bridge/ipcChannels'
import { executeEventTask } from './main/events/eventExecutor'
import { EventTaskStore } from './main/storage/eventTaskStore'
import { listSerialPorts } from './main/serial/serialScanner'
import { SerialService } from './main/serial/serialService'
import { createMainWindow } from './main/windows/mainWindow'
import {
	createAppTray,
	setTrayLocale,
	refreshAppTray,
} from './main/windows/tray'
import { normalizeLocale } from './shared/i18n'
import type { StableEvent } from './shared/types/sensenode'

if (started) {
	app.quit()
}

app.setAppUserModelId('SenseNode')

let mainWindow: BrowserWindow | null = null
let isQuitting = false
const serialService = new SerialService()
const eventTaskStore = new EventTaskStore()
const eventThrottle = new Map<string, number>()
const taskThrottle = new Map<string, number>()

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
	ipcMain.handle(ipcChannels.eventTasksList, () => eventTaskStore.list())
	ipcMain.handle(ipcChannels.eventTasksUpsert, (_event, task) => {
		try {
			return eventTaskStore.upsert(task)
		} catch (error) {
			console.error('[main] event task upsert failed:', error)
			throw error
		}
	})
	ipcMain.handle(ipcChannels.eventTasksRemove, (_event, id: string) => {
		try {
			eventTaskStore.remove(id)
		} catch (error) {
			console.error('[main] event task remove failed:', error)
			throw error
		}
	})
	ipcMain.handle(
		ipcChannels.eventNotifyStable,
		(_event, stableEvent: string) => {
			handleStableEvent(stableEvent)
		},
	)
}

function handleStableEvent(stableEvent: string): void {
	const now = Date.now()
	const lastAt = eventThrottle.get(stableEvent) ?? 0
	if (now - lastAt < 2000) return
	eventThrottle.set(stableEvent, now)
	const tasks = eventTaskStore.findByEvent(stableEvent)
	for (const task of tasks) {
		const lastRun = taskThrottle.get(task.id) ?? 0
		if (task.cooldownMs > 0 && now - lastRun < task.cooldownMs) {
			continue
		}
		taskThrottle.set(task.id, now)
		executeEventTask(task, stableEvent)
	}
}

function showMainWindow(): void {
	mainWindow?.show()
	mainWindow?.focus()
}

function createWindows(): void {
	mainWindow = createMainWindow()

	mainWindow.webContents.on('context-menu', (_event, params) => {
		const menu = Menu.buildFromTemplate([
			{
				label: '复制',
				role: 'copy',
				enabled: Boolean(params.selectionText),
			},
			{
				label: '粘贴',
				role: 'paste',
				enabled: params.editFlags.canPaste,
			},
			{
				label: '全选',
				role: 'selectAll',
			},
		])

		menu.popup({ window: mainWindow ?? undefined })
	})

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
