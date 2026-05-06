<script setup lang="ts">
import { computed } from 'vue'
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'
import { useSerialStore } from '../../stores/serialStore'

const device = useDeviceStore()
const serial = useSerialStore()
const { t } = useI18n()

const statusLabel = computed(() =>
	device.currentFrame
		? t(`serial.status.${serial.status.state}`)
		: t('device.notConnected'),
)

const hint = computed(() => {
	if (!device.currentFrame) return t('device.notConnected')
	if (device.stableEvent === 'tap') return t('hud.tapDetected')
	if (device.stableEvent === 'hand_near') return t('hud.handNear')
	if (device.stableEvent === 'hand_hover') return t('hud.handHover')
	return ''
})
</script>

<template>
	<UButton
		class="flex h-screen w-screen flex-col justify-between overflow-hidden bg-default/85 px-[14px] py-[13px] text-left text-default backdrop-blur-[20px] [-webkit-app-region:drag]"
		color="neutral"
		variant="soft"
		type="button"
		@click="window.sensenode.app.showMainWindow()"
	>
		<div
			class="mb-2 flex items-center justify-between text-[13px] leading-[1.2]"
		>
			<span class="font-medium">{{ t('app.title') }}</span>
			<span class="font-['JetBrains_Mono','MiSans',monospace]">{{
				statusLabel
			}}</span>
		</div>
		<div class="font-['JetBrains_Mono','MiSans',monospace] text-muted">
			{{ t('hud.event') }}:
			{{ device.currentFrame ? device.stableEvent : '-' }} ·
			{{ t('scene.tofMm') }}: {{ device.currentFrame?.tof_mm ?? '-' }}mm
		</div>
		<div
			class="font-['JetBrains_Mono','MiSans',monospace] text-[12px] leading-[1.2] text-highlighted"
		>
			{{
				hint ||
				`${t('hud.confidence')} ${device.currentFrame?.confidence?.toFixed(2) ?? '-'}`
			}}
		</div>
	</UButton>
</template>
