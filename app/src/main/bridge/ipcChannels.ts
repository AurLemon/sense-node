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
	appSetLocale: 'sensenode:app:set-locale',
} as const
