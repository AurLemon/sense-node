import { BrowserWindow } from 'electron'
import path from 'node:path'
import { getAppIcon } from './appIcon'

export function createMainWindow(): BrowserWindow {
	const window = new BrowserWindow({
		width: 1180,
		height: 760,
		minWidth: 960,
		minHeight: 640,
		backgroundColor: '#080A0F',
		icon: getAppIcon(),
		title: 'SenseNode',
		webPreferences: {
			preload: path.join(__dirname, 'preload.js'),
			contextIsolation: true,
			nodeIntegration: false,
			sandbox: false,
		},
	})

	if (MAIN_WINDOW_VITE_DEV_SERVER_URL) {
		void window.loadURL(MAIN_WINDOW_VITE_DEV_SERVER_URL)
	} else {
		void window.loadFile(
			path.join(__dirname, `../renderer/${MAIN_WINDOW_VITE_NAME}/index.html`),
		)
	}

	return window
}
