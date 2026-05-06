<script setup lang="ts">
import { computed, ref } from 'vue'
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'
import { useSettingsStore } from '../../stores/settingsStore'

const device = useDeviceStore()
const settings = useSettingsStore()
const { t } = useI18n()
const prompt = ref('请总结当前 SenseNode 的状态。')
const result = ref('')

const summary = computed(() => {
	return device.frames
		.slice(0, 200)
		.map((frame) => frame.final_event ?? frame.type)
		.slice(0, 12)
		.join(' -> ')
})

function analyze(): void {
	result.value = `${t('lm.reserved')}: ${settings.llmConfig.provider}. ${t('lm.currentStable')}: ${device.stableEvent}.`
}
</script>

<template>
	<div class="ai-page">
		<div class="panel form">
			<label>
				{{ t('lm.provider') }}
				<select v-model="settings.llmConfig.provider" class="input">
					<option value="none">none</option>
					<option value="ollama">ollama</option>
					<option value="openai-compatible">openai-compatible</option>
				</select>
			</label>
			<label>
				{{ t('lm.endpoint') }}
				<input v-model="settings.llmConfig.endpoint" class="input" />
			</label>
			<label>
				{{ t('lm.model') }}
				<input v-model="settings.llmConfig.model" class="input" />
			</label>
			<label class="wide">
				{{ t('lm.prompt') }}
				<textarea v-model="prompt" class="input textarea" />
			</label>
			<label class="wide">
				{{ t('lm.summary') }}
				<textarea class="input textarea" readonly :value="summary" />
			</label>
			<button class="button" type="button" @click="analyze">
				{{ t('lm.analyze') }}
			</button>
			<div class="mono muted">{{ result }}</div>
		</div>
	</div>
</template>

<style scoped>
.ai-page {
	height: 100%;
}

.form {
	display: grid;
	grid-template-columns: repeat(3, minmax(0, 1fr));
	gap: 12px;
	padding: 12px;
}

label {
	display: grid;
	gap: 7px;
	color: var(--color-text-muted);
	font-size: 12px;
}

.wide {
	grid-column: 1 / -1;
}

.textarea {
	min-height: 90px;
	padding: 10px;
	resize: vertical;
}
</style>
