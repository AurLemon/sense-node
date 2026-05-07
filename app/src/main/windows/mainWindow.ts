import { BrowserWindow } from 'electron'
import path from 'node:path'
import { getAppIcon } from './appIcon'

export function createMainWindow(): BrowserWindow {
	const window = new BrowserWindow({
		width: 500,
		height: 700,
		minWidth: 500,
		minHeight: 700,
		backgroundColor: '#FAFAFA',
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
