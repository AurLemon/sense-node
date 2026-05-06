import { contextBridge, ipcRenderer } from 'electron'
import { ipcChannels } from './main/bridge/ipcChannels'
import type {
	DataSourceStatus,
	SenseNodeFrame,
	SerialLineEntry,
	SerialPortInfo,
} from './shared/types/sensenode'

function subscribe<T>(
	channel: string,
	callback: (payload: T) => void,
): () => void {
	const listener = (_event: Electron.IpcRendererEvent, payload: T) =>
		callback(payload)
	ipcRenderer.on(channel, listener)
	return () => ipcRenderer.removeListener(channel, listener)
}

contextBridge.exposeInMainWorld('sensenode', {
	serial: {
		listPorts: (): Promise<SerialPortInfo[]> =>
			ipcRenderer.invoke(ipcChannels.serialListPorts),
		connect: (path: string, baudRate?: number): Promise<DataSourceStatus> =>
			ipcRenderer.invoke(ipcChannels.serialConnect, path, baudRate),
		disconnect: (): Promise<DataSourceStatus> =>
			ipcRenderer.invoke(ipcChannels.serialDisconnect),
		pause: (): Promise<DataSourceStatus> =>
			ipcRenderer.invoke(ipcChannels.serialPause),
		resume: (): Promise<DataSourceStatus> =>
			ipcRenderer.invoke(ipcChannels.serialResume),
		onLine: (callback: (line: SerialLineEntry) => void): (() => void) =>
			subscribe(ipcChannels.serialLine, callback),
	},
	device: {
		onFrame: (callback: (frame: SenseNodeFrame) => void): (() => void) =>
			subscribe(ipcChannels.deviceFrame, callback),
		onStatus: (callback: (status: DataSourceStatus) => void): (() => void) =>
			subscribe(ipcChannels.dataSourceStatus, callback),
	},
	app: {
		showMainWindow: (): Promise<void> =>
			ipcRenderer.invoke(ipcChannels.appShowMainWindow),
		showHud: (): Promise<void> => ipcRenderer.invoke(ipcChannels.appShowHud),
		hideHud: (): Promise<void> => ipcRenderer.invoke(ipcChannels.appHideHud),
		setLocale: (locale: string): Promise<void> =>
			ipcRenderer.invoke(ipcChannels.appSetLocale, locale),
	},
})
