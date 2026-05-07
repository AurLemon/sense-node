import { Menu, Notification, Tray, app } from 'electron'
import type { SerialService } from '../serial/serialService'
import type { Locale } from '../../shared/i18n'
import { normalizeLocale } from '../../shared/i18n'
import type { StableEvent } from '../../shared/types/sensenode'
import { getAppIcon } from './appIcon'

let tray: Tray | null = null
let currentLocale: Locale = normalizeLocale(app.getLocale())

const trayLabels = {
	'zh-CN': {
		showMainWindow: '显示主窗口',
		pauseSerial: '暂停串口',
		resumeSerial: '恢复串口',
		quit: '退出',
	},
	'en-US': {
		showMainWindow: 'Show Main Window',
		pauseSerial: 'Pause Serial',
		resumeSerial: 'Resume Serial',
		quit: 'Quit',
	},
} as const

const eventLabels = {
	'zh-CN': {
		idle: '空闲',
		tap: '点击',
		board_motion: '板载运动',
		hand_hover: '手悬停',
		hand_near: '手靠近',
		hand_leave: '手离开',
		reject: '拒绝',
		unknown: '未知',
	},
	'en-US': {
		idle: 'Idle',
		tap: 'Tap',
		board_motion: 'Board motion',
		hand_hover: 'Hand hover',
		hand_near: 'Hand near',
		hand_leave: 'Hand leave',
		reject: 'Reject',
		unknown: 'Unknown',
	},
} as const

function getTrayLabels(locale: Locale) {
	return trayLabels[locale] ?? trayLabels['en-US']
}

function refreshTrayMenu(args: {
	showMainWindow: () => void
	serialService: SerialService
	onQuit: () => void
}): void {
	if (!tray) return

	const labels = getTrayLabels(currentLocale)
	const menu = Menu.buildFromTemplate([
		{ label: labels.showMainWindow, click: () => args.showMainWindow() },
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
	serialService: SerialService
	onQuit: () => void
}): void {
	refreshTrayMenu(args)
}

export function showTrayEventNotification(title: string, body: string): void {
	if (!tray) return
	tray.displayBalloon({
		title,
		content: body,
		iconType: 'info',
	})
}

export function showGlobalEventNotification(title: string, body: string): void {
	if (!Notification.isSupported()) return
	new Notification({ title, body }).show()
}

export function getEventLabel(locale: Locale, event: StableEvent): string {
	return eventLabels[locale]?.[event] ?? event
}
