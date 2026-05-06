import { BrowserWindow, screen } from 'electron'
import path from 'node:path'

export function createHudWindow(): BrowserWindow {
	const display = screen.getPrimaryDisplay()
	const width = 280
	const height = 96
	const margin = 18
	const window = new BrowserWindow({
		width,
		height,
		x: display.workArea.x + display.workArea.width - width - margin,
		y: display.workArea.y + display.workArea.height - height - margin,
		frame: false,
		resizable: false,
		alwaysOnTop: true,
		skipTaskbar: true,
		transparent: true,
		show: false,
		backgroundColor: '#00000000',
		webPreferences: {
			preload: path.join(__dirname, 'preload.js'),
			contextIsolation: true,
			nodeIntegration: false,
			sandbox: false,
		},
	})

	if (MAIN_WINDOW_VITE_DEV_SERVER_URL) {
		void window.loadURL(`${MAIN_WINDOW_VITE_DEV_SERVER_URL}#hud`)
	} else {
		void window.loadFile(
			path.join(__dirname, `../renderer/${MAIN_WINDOW_VITE_NAME}/index.html`),
			{ hash: 'hud' },
		)
	}

	return window
}
