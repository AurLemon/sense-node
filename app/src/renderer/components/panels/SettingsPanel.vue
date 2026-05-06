<script setup lang="ts">
import DropdownSelect from '../ui/DropdownSelect.vue'
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'
import { useSettingsStore } from '../../stores/settingsStore'

const device = useDeviceStore()
const settings = useSettingsStore()
const { t } = useI18n()
</script>

<template>
	<div class="settings">
		<div class="panel form">
			<label>
				{{ t('common.language') }}
				<DropdownSelect
					v-model="settings.locale"
					:items="[
						{ value: 'zh-CN', label: t('locale.zh') },
						{ value: 'en-US', label: t('locale.en') },
					]"
				/>
			</label>
			<label>
				{{ t('common.theme') }}
				<DropdownSelect
					v-model="settings.themeMode"
					:items="[
						{ value: 'dark', label: t('theme.dark') },
						{ value: 'light', label: t('theme.light') },
					]"
				/>
			</label>
			<label class="check">
				<input v-model="settings.miniHudEnabled" type="checkbox" />
				{{ t('settings.miniHud') }}
			</label>
			<label class="check">
				<input v-model="settings.autoReconnect" type="checkbox" />
				{{ t('settings.autoReconnect') }}
			</label>
			<label class="check">
				<input v-model="settings.eventStabilizerEnabled" type="checkbox" />
				{{ t('settings.stabilizer') }}
			</label>
			<label>
				{{ t('settings.accent') }}
				<input v-model="settings.themeAccent" class="input" type="color" />
			</label>
		</div>

		<div class="panel mono note">
			{{ t('settings.currentStable') }}:
			{{ device.currentFrame ? device.stableEvent : '-' }}
		</div>
	</div>
</template>

<style scoped>
.settings {
	display: grid;
	gap: 12px;
}

.form {
	display: flex;
	flex-wrap: wrap;
	gap: 12px;
	padding: 12px;
}

label {
	display: grid;
	gap: 7px;
	min-width: 180px;
	color: var(--color-text-muted);
	font-size: 12px;
}

.check {
	display: inline-flex;
	align-items: center;
	min-height: 34px;
}

.note {
	padding: 12px;
	color: var(--color-text-muted);
}
</style>
