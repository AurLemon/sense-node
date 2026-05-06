export const ipcChannels = {
	serialListPorts: 'sensenode:serial:list-ports',
	serialConnect: 'sensenode:serial:connect',
	serialDisconnect: 'sensenode:serial:disconnect',
	serialPause: 'sensenode:serial:pause',
	serialResume: 'sensenode:serial:resume',
	deviceFrame: 'sensenode:device:frame',
	serialLine: 'sensenode:serial:line',
	dataSourceStatus: 'sensenode:datasource:status',
	appShowMainWindow: 'sensenode:app:show-main-window',
	appShowHud: 'sensenode:app:show-hud',
	appHideHud: 'sensenode:app:hide-hud',
	appSetLocale: 'sensenode:app:set-locale',
} as const
