import { Menu, Tray, app } from 'electron'
import type { SerialService } from '../serial/serialService'
import type { Locale } from '../../shared/i18n'
import { normalizeLocale } from '../../shared/i18n'
import { getAppIcon } from './appIcon'

let tray: Tray | null = null
let currentLocale: Locale = normalizeLocale(app.getLocale())

const trayLabels = {
	'zh-CN': {
		showMainWindow: '显示主窗口',
		hideHud: '隐藏 HUD',
		showHud: '显示 HUD',
		pauseSerial: '暂停串口',
		resumeSerial: '恢复串口',
		quit: '退出',
	},
	'en-US': {
		showMainWindow: 'Show Main Window',
		hideHud: 'Hide HUD',
		showHud: 'Show HUD',
		pauseSerial: 'Pause Serial',
		resumeSerial: 'Resume Serial',
		quit: 'Quit',
	},
} as const

function getTrayLabels(locale: Locale) {
	return trayLabels[locale] ?? trayLabels['en-US']
}

function refreshTrayMenu(args: {
	showMainWindow: () => void
	showHudWindow: () => void
	hideHudWindow: () => void
	serialService: SerialService
	onQuit: () => void
}): void {
	if (!tray) return

	const labels = getTrayLabels(currentLocale)
	const menu = Menu.buildFromTemplate([
		{ label: labels.showMainWindow, click: () => args.showMainWindow() },
		{ label: labels.hideHud, click: () => args.hideHudWindow() },
		{ label: labels.showHud, click: () => args.showHudWindow() },
		{ type: 'separator' },
		{ label: labels.pauseSerial, click: () => args.serialService.pause() },
		{ label: labels.resumeSerial, click: () => args.serialService.resume() },
		{ type: 'separator' },
		{
			label: labels.quit,
			click: () => {
				args.onQuit()
				app.quit()
			},
		},
	])
	tray.setContextMenu(menu)
}

export function createAppTray(args: {
	showMainWindow: () => void
	showHudWindow: () => void
	hideHudWindow: () => void
	serialService: SerialService
	onQuit: () => void
}): Tray {
	const image = getAppIcon()
	tray = new Tray(image)
	tray.setToolTip('SenseNode')
	tray.on('click', () => args.showMainWindow())
	refreshTrayMenu(args)
	return tray
}

export function setTrayLocale(locale: Locale): void {
	currentLocale = normalizeLocale(locale)
}

export function refreshAppTray(args: {
	showMainWindow: () => void
	showHudWindow: () => void
	hideHudWindow: () => void
	serialService: SerialService
	onQuit: () => void
}): void {
	refreshTrayMenu(args)
}
