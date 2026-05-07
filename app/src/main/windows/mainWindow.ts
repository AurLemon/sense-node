import { BrowserWindow } from 'electron'
import path from 'node:path'
import { getAppIcon } from './appIcon'

export function createMainWindow(): BrowserWindow {
	const window = new BrowserWindow({
		width: 450,
		height: 700,
		minWidth: 450,
		minHeight: 700,
		maxWidth: 550,
		maxHeight: 850,
		backgroundColor: '#FAFAFA',
		icon: getAppIcon(),
		title: 'SenseNode',
		autoHideMenuBar: true,
		webPreferences: {
			preload: path.join(__dirname, 'preload.js'),
			contextIsolation: true,
			nodeIntegration: false,
			sandbox: false,
		},
	})

	if (process.platform === 'win32' || process.platform === 'linux') {
		window.setMenuBarVisibility(false)
		window.setMenu(null)
	}

	if (MAIN_WINDOW_VITE_DEV_SERVER_URL) {
		void window.loadURL(MAIN_WINDOW_VITE_DEV_SERVER_URL)
	} else {
		void window.loadFile(
			path.join(__dirname, `../renderer/${MAIN_WINDOW_VITE_NAME}/index.html`),
		)
	}

	return window
}
